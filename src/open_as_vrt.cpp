#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>

// done:
//   - augment extent
//   - augment projection

// todo:
//  - multiple dsn (only allowed by the warper, not conversion to vrt)
//  - sds selection, by argument, by name, by default the first one

GDALDataset *gdal_open_dsn(CharacterVector dsn) {
  GDALAllRegister();
  auto DS = GDALDataset::Open(dsn[0], GDAL_OF_RASTER | GDAL_OF_SHARED,
                                   nullptr, nullptr, nullptr);
  if( DS == nullptr )
  {
    return nullptr;
  }
  return GDALDataset::FromHandle( DS);
}
CharacterVector list_subdatasets(GDALDataset *poDataset) {
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

  int dscount = 0;
  if (has_sds) {
    // owned by the object
    char **SDS = GDALGetMetadata(poDataset, "SUBDATASETS");
    int sdi = 0;
    while (SDS && SDS[sdi] != NULL) {
      sdi++; // count
    }
    dscount = sdi;
  }
  if (dscount < 1) return "";

  // we only want the first of each pair
  dscount = dscount / 2;
  Rcpp::CharacterVector ret(dscount);
  if (has_sds) {
    // we have subdatasets, so list them all
    // owned by the object
    char **SDS2 = GDALGetMetadata(poDataset, "SUBDATASETS");
    for (int ii = 0; ii < dscount; ii++) {
      // ii*2 because SDS tokens come in pairs
      char  **papszTokens = CSLTokenizeString2(SDS2[ii * 2], "=", 0);

      ret(ii) = papszTokens[1];
      CSLDestroy( papszTokens );
    }
  }
 return ret;
}
bool default_geotransform(GDALDataset *poDataset) {
  bool test = false;
  double geotransform[6];
  poDataset->GetGeoTransform(geotransform);
  test =
    geotransform[0] == 0.0 &&
    geotransform[1] == 1.0 &&
    geotransform[2] == 0.0 &&
    geotransform[3] == 0.0 &&
    geotransform[4] == 0.0 &&
    geotransform[5] == 1.0;

  return test;
}
GDALDataset *gdal_dataset_augment(CharacterVector dsn,
                                  NumericVector extent,
                                  CharacterVector projection,
                                  IntegerVector sds) {

  auto poSrcDS = gdal_open_dsn(dsn);
  if( poSrcDS == nullptr )
   {
     return nullptr;
  }
  CPLStringList argv;
  argv.AddString("-of");
  argv.AddString("VRT");
  bool set_extent = extent.size() == 4;
  bool set_projection = !projection[0].empty();
  bool set_subdataset;
  int isds = sds[0];
  if (poSrcDS->GetRasterXSize() == 512 && poSrcDS->GetRasterYSize() == 512 && default_geotransform(poSrcDS)) {
    // we *strongly* suspect
    // need error handling on this is so janky
    CharacterVector sdsnames = list_subdatasets(poSrcDS);
    if (sdsnames.length() > 0 && !sdsnames[0].empty()) {
      // user asked for 1-based SDS, or zero was default
      if (isds < 1) {
        isds = 1;
      }
      if (isds > (sdsnames.length())) {
        return nullptr;
      }

      poSrcDS->ReleaseRef();
      CharacterVector subdsn(1);
      subdsn[0] = sdsnames[isds - 1];
      poSrcDS = gdal_open_dsn(subdsn);

    }
  }



  if (set_extent) {
    if ((extent[1] <= extent[0]) || extent[3] <= extent[2]) {
      poSrcDS->ReleaseRef();
      Rprintf("extent must be valid c(xmin, xmax, ymin, ymax)\n");
      return nullptr;
    }
    argv.AddString("-a_ullr");
    argv.AddString(CPLSPrintf("%f", extent[0]));
    argv.AddString(CPLSPrintf("%f", extent[3]));
    argv.AddString(CPLSPrintf("%f", extent[1]));
    argv.AddString(CPLSPrintf("%f", extent[2]));
  }
  if (set_projection) {
     argv.AddString("-a_srs");
     argv.AddString(projection[0]);
  }
  GDALTranslateOptions* psOptions = GDALTranslateOptionsNew(argv.List(), nullptr);
  auto hRet = GDALTranslate("", GDALDataset::ToHandle(poSrcDS),
                            psOptions, nullptr);
  GDALTranslateOptionsFree( psOptions );
  poSrcDS->ReleaseRef();

  return GDALDataset::FromHandle(hRet);
}


// [[Rcpp::export]]
CharacterVector open_as_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection, IntegerVector sds) {
  CharacterVector out(1);

  GDALDataset *poDATASET = gdal_dataset_augment(dsn, extent, projection, sds);

  auto poDS = cpl::down_cast<VRTDataset*>(GDALDataset::FromHandle(poDATASET));
  if( poDS )
  {
    poDS->SetDescription(CPLSPrintf("%s", dsn[0]));
    poDS->SetWritable(false);
  }
  if (poDS) {
    const char *xmlvrt = poDS->GetMetadata("xml:VRT")[0];

    out[0] = xmlvrt;
    poDS->ReleaseRef();
  }
  return out;
}
