#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>



// topos <- c("/vsicurl/https://public.services.aad.gov.au/datasets/science/GEBCO_2019_GEOTIFF/GEBCO_2019.tif",
//            "/vsicurl/https://opentopography.s3.sdsc.edu/raster/NASADEM/NASADEM_be.vrt")
//
//   file <-"../vapour/inst/extdata/volcano.tif"
// gdalmin:::open_gdal(file)
//   gdalmin:::open_gdal(c(file, topos))
//


bool has_subdataset0(GDALDataset *poDataset) {
  // not faster, we good
  //if (poDataset->GetRasterCount() > 0) return false;
  char **MDdomain = GDALGetMetadataDomainList(poDataset);
  int mdi = 0; // iterate though MetadataDomainList
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);
  return has_sds;
}

CharacterVector list_subdataset_i(GDALDataset *poDataset, int i_sds) {
  // need error handling on this is so janky
  int sdi = 0;
  // don't call this function without calling has_subdatasets() first
  // owned by the object
  char **SDS2 = poDataset->GetMetadata("SUBDATASETS");
  while (SDS2 && SDS2[sdi] != NULL) {
    sdi++; // count
  }
  // FIXME
  if (sdi < 1) return "";
  if (!(sdi % 2 == 0)) return "";
  // we only want the first of each pair
  int dscount = sdi / 2;
  CharacterVector ret(1);
  if (dscount > 0) {
    // we have subdatasets, so loop but just grab i-th
    for (int ii = 0; ii < dscount; ii++) {
      if (ii == (i_sds - 1)) {
        // ii*2 because SDS tokens come in pairs
        char  **papszTokens = CSLTokenizeString2(SDS2[ii * 2], "=", 0);
        ret[0] = papszTokens[1];
        CSLDestroy( papszTokens );
      }
    }
  }
  return ret;
}
GDALDatasetH open_gdalH(const char * dsn, IntegerVector sds) {
  GDALAllRegister();
  GDALDatasetH DS = GDALOpenShared(dsn, GA_ReadOnly);
  if (sds[0] > 0 && has_subdataset0((GDALDataset*) DS)) {
    CharacterVector sdsnames = list_subdataset_i((GDALDataset*)DS, sds[0]);
    if (sdsnames.length() > 0 && !sdsnames[0].empty()) {
      // user asked for specific 1-based SDS, which we obtained
      // ///user asked for 1-based SDS, or zero was default (we don't open SDS)
      // if (sds[0] > (sdsnames.length())) {
      //   return nullptr;
      // }
      GDALClose((GDALDataset*) DS);
      DS = GDALOpenShared(sdsnames[0], GA_ReadOnly);
   }
  //if (DS == nullptr) stop("cannot open %s\n", dsn[0]);

  }
  return DS;
}
GDALDatasetH* open_gdalH_multiple(CharacterVector dsn, IntegerVector sds) {
  GDALAllRegister();
  GDALDatasetH* poHDS;
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * dsn.size()));
  for (int i = 0; i < dsn.size(); i++) poHDS[i] = open_gdalH(dsn[i], sds);
  return poHDS;
}
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


// [[Rcpp::export]]
CharacterVector open_xml(CharacterVector dsn) {
  GDALAllRegister();
  CharacterVector out(1);

  GDALDataset *poSrcDS = (GDALDataset *)GDALOpenShared(dsn[0], GA_ReadOnly);
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
  GDALClose((GDALDatasetH) poSrcDS);
  return out;
}


// [[Rcpp::export]]
IntegerVector open_gdal(CharacterVector source_filename) {
  NumericVector source_extent(4);
  source_extent[0] = 0;
  source_extent[1] = 1;
  source_extent[2] = 0;
  source_extent[3] = 2;
  CharacterVector source_WKT(1);
  source_WKT[0] = "OGC:CRS84";

  GDALAllRegister();
  CPLStringList translate_argv;
  translate_argv.AddString("-of");
  translate_argv.AddString("VRT");


  bool set_extent = false;
  bool set_projection = false;

  if (set_extent) {
    if ((source_extent[1] <= source_extent[0]) || source_extent[3] <= source_extent[2]) {
      Rprintf("source_extent must be valid c(xmin, xmax, ymin, ymax)\n");
      stop("error: gdal_warp_in_memory");
    }
    translate_argv.AddString("-a_ullr");
    translate_argv.AddString(CPLSPrintf("%f", source_extent[0]));
    translate_argv.AddString(CPLSPrintf("%f", source_extent[3]));
    translate_argv.AddString(CPLSPrintf("%f", source_extent[1]));
    translate_argv.AddString(CPLSPrintf("%f", source_extent[2]));
  }
  if (set_projection) {
    translate_argv.AddString("-a_srs");
    translate_argv.AddString(source_WKT[0]);
  }

  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(translate_argv.List(), nullptr);

  GDALDatasetH *poSrcDS;
  GDALDataset *DS;

  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * source_filename.size()));

  //CharacterVector source_vrt(source_filename.size());

  for (int i = 0; i < source_filename.size(); i++) {
    DS = (GDALDataset *) GDALOpenShared(source_filename[i], GA_ReadOnly);
    // unwind everything, and stop
    if (DS == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose( poSrcDS[j]);
      }
      GDALTranslateOptionsFree( psTransOptions );
      CPLFree(poSrcDS);
      Rprintf("input source not readable: %s\n", (char *)source_filename[i]);
      Rcpp::stop("");
    }
    if (set_extent || set_projection) {
      // GDALDataset *h1;
      // h1 = (GDALDataset *)GDALTranslate("", DS, psTransOptions, nullptr);

      poSrcDS[i] = GDALTranslate("", DS, psTransOptions, nullptr);
      GDALClose((GDALDatasetH) DS);
    } else {
      poSrcDS[i] = (GDALDatasetH) DS;
    }
  }
  GDALTranslateOptionsFree( psTransOptions );

  for (int i = 0; i < source_filename.size(); i++) GDALClose((GDALDatasetH) poSrcDS[i]);
  CPLFree(poSrcDS);
  return 1;
}
