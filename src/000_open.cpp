#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>

// for GHRSST this is what works
// "NETCDF:\"https://podaac-opendap.jpl.nasa.gov/opendap/allData/ghrsst/data/GDS2/L4/GLOB/JPL/MUR/v4.1/2022/114/20220424090000-JPL-L4_GHRSST-SSTfnd-MUR-GLOB-v02.0-fv04.1.nc\":analysed_sst"
//  netcdf: https://podaac-opendap.jpl.nasa.gov/opendap/allData/ghrsst/data/GDS2/L4/GLOB/JPL/MUR/v4.1/2022/114/20220424090000-JPL-L4_GHRSST-SSTfnd-MUR-GLOB-v02.0-fv04.1.nc

// topos <- c("/vsicurl/https://public.services.aad.gov.au/datasets/science/GEBCO_2019_GEOTIFF/GEBCO_2019.tif",
//            "/vsicurl/https://opentopography.s3.sdsc.edu/raster/NASADEM/NASADEM_be.vrt")
//
//   file <-"../vapour/inst/extdata/volcano.tif"
// gdalmin:::open_gdal(file)
//   gdalmin:::open_gdal(c(file, topos))
//




// does it have subdatasets?
bool has_subdataset0(GDALDataset *poDataset) {
  // not faster, we good
  //if (poDataset->GetRasterCount() > 0) return false;
  char **MDdomain = GDALGetMetadataDomainList(poDataset);
  int mdi = 0; // iterate though MetadataDomainList
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
      //cleanup
      CSLDestroy(MDdomain);
      return has_sds;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);
  return has_sds;
}

// obtain the i-th subdataset *name* (i is 1-based)
CharacterVector list_subdataset_i(GDALDataset *poDataset, int i_sds) {
  // need error handling on this is so janky
  int sdi = 0;
  // don't call this function without calling has_subdatasets() first
  // owned by the object
  CharacterVector ret(1);

  char **SDS2 = poDataset->GetMetadata("SUBDATASETS");
  while (SDS2 && SDS2[sdi] != NULL) {
    if (sdi * 2 == (i_sds - 1)) {
      // ii*2 because SDS tokens come in pairs
      char  **papszTokens = CSLTokenizeString2(SDS2[sdi * 2], "=", 0);
      ret[0] = papszTokens[1];
      CSLDestroy( papszTokens );
      break;
    }
     sdi++; // count
  }
  return ret;
}

// open the DSN, open a subdataset if sds > 0 (else you get the outer shell)
GDALDatasetH open_gdalH(const char * dsn, IntegerVector sds) {
  GDALAllRegister();
  GDALDatasetH DS = GDALOpen(dsn, GA_ReadOnly);
  if (sds[0] > 0 && has_subdataset0((GDALDataset*) DS)) {
    CharacterVector sdsnames = list_subdataset_i((GDALDataset*)DS, sds[0]);
    if (sdsnames.length() > 0 && !sdsnames[0].empty()) {
      GDALClose((GDALDataset*) DS);
      DS = GDALOpen(sdsnames[0], GA_ReadOnly);
   }

  }
  return DS;
}

// open the DSN/s with open_gdalH(), possibly with a sds (1-based)
// (nothing uses this function)
GDALDatasetH* open_gdalH_multiple(CharacterVector dsn, IntegerVector sds) {
  GDALAllRegister();
  GDALDatasetH* poHDS;
  // whoever calls this will have to CPLFree() this
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * dsn.size()));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = open_gdalH(dsn[i], sds);
  return poHDS;
}

// open the DSN with open_gdalH() but translate it to VRT with extent (=a_ullr) and/or projection (=a_srs)
GDALDatasetH open_gdalH_atranslate(const char* dsn, NumericVector extent, CharacterVector projection, IntegerVector sds) {
  GDALAllRegister();
  CPLStringList translate_argv;
  translate_argv.AddString("-of");
  translate_argv.AddString("VRT");
  if (extent.size() == 4) {
    translate_argv.AddString("-a_ullr");
    translate_argv.AddString(CPLSPrintf("%f", extent[0]));
    translate_argv.AddString(CPLSPrintf("%f", extent[3]));
    translate_argv.AddString(CPLSPrintf("%f", extent[1]));
    translate_argv.AddString(CPLSPrintf("%f", extent[2]));
  }
  if (!projection[0].empty()) {
    translate_argv.AddString("-a_srs");
    translate_argv.AddString(projection[0]);
  }

  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(translate_argv.List(), nullptr);
  GDALDatasetH a_DS = GDALTranslate("", (GDALDataset*)open_gdalH(dsn, sds), psTransOptions, nullptr);
  GDALTranslateOptionsFree( psTransOptions );
  return a_DS;
}

// convert an opened GDALDataset to chunk-of-text VRT, if it is VRT you get it direct
//  if it's a different driver it is firs CreateCopy()ied to VRT
const char* gdal_vrt_text(GDALDataset* poSrcDS) {
  CharacterVector out(1);
  // can't do this unless poSrcDS really is VRT
  if (EQUAL(poSrcDS->GetDriverName(),  "VRT")) {
    VRTDataset * VRTdcDS = cpl::down_cast<VRTDataset *>(poSrcDS );
    if (!(VRTdcDS == nullptr)) out[0] = VRTdcDS->GetMetadata("xml:VRT")[0];
  } else {
    GDALDriver *poDriver = (GDALDriver *)GDALGetDriverByName("VRT");
    GDALDataset *VRTDS = poDriver->CreateCopy("", poSrcDS, false, NULL, NULL, NULL);
    if (!(VRTDS == nullptr)) out[0] = VRTDS->GetMetadata("xml:VRT")[0];
    GDALClose((GDALDatasetH) VRTDS);
  }
  return out[0];
}

// open any dataset *name* (file,url,sdsstring,etc.) to chunk-of-VRT text
// optionally with extent(=a_ullr) and/or projection(=a_srs) augmented
// [[Rcpp::export]]
CharacterVector open_to_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection, IntegerVector sds) {
  CharacterVector out(dsn.size());
  GDALDatasetH DS;
  for (int i = 0; i < out.size(); i++) {
    if (extent.size() == 4 || (!projection[0].empty())) {
      DS = open_gdalH_atranslate(dsn[0], extent, projection, sds);
    } else {
      DS = open_gdalH(dsn[0], sds);
    }
    out[i] = gdal_vrt_text((GDALDataset*) DS);
    GDALClose(DS);
  }
  return out;
}




