#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>

GDALDataset *gdal_dataset_augment(CharacterVector dsn,
                                  NumericVector extent,
                                  CharacterVector projection) {
  GDALAllRegister();
  auto poSrcDS = GDALDataset::Open(dsn[0], GDAL_OF_RASTER | GDAL_OF_SHARED,
                                   nullptr, nullptr, nullptr);
  if( poSrcDS == nullptr )
  {
    return nullptr;
  }
  CPLStringList argv;
  argv.AddString("-of");
  argv.AddString("VRT");
  bool set_extent = extent.size() == 4;
  bool set_projection = !projection[0].empty();
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
  auto poDS = cpl::down_cast<VRTDataset*>(GDALDataset::FromHandle(hRet));
  if( poDS )
  {
    poDS->SetDescription(CPLSPrintf("%s", dsn[0]));
    poDS->SetWritable(false);
  }
  return poDS;
}


// [[Rcpp::export]]
CharacterVector open_as_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection) {
  CharacterVector out(1);

  GDALDataset *poDS = gdal_dataset_augment(dsn, extent, projection);
  if (poDS) {
    const char *xmlvrt = poDS->GetMetadata("xml:VRT")[0];

    out[0] = xmlvrt;
    poDS->ReleaseRef();
  }
  return out;
}
