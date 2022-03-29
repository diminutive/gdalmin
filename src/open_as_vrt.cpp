#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>

// [[Rcpp::export]]
NumericVector open_as_vrt(CharacterVector dsn, NumericVector a_ullr) {


  GDALAllRegister();
  //  GDALDataset  *poDataset;
  //  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_ReadOnly );

  auto poSrcDS = GDALDataset::Open(dsn[0], GDAL_OF_RASTER | GDAL_OF_SHARED,
                                   nullptr, nullptr, nullptr);
  double        adfGeoTransform[6];
  adfGeoTransform[0] = NA_REAL;
  adfGeoTransform[1] = NA_REAL;
  adfGeoTransform[2] = NA_REAL;
  adfGeoTransform[3] = NA_REAL;
  adfGeoTransform[4] = NA_REAL;
  adfGeoTransform[5] = NA_REAL;

  if( poSrcDS == nullptr )
  {
    return NA_REAL;
  }

  CPLStringList argv;

  argv.AddString("-of");
  argv.AddString("VRT");

  bool bbox = true;
  if (bbox) {
    argv.AddString("-a_ullr");
    argv.AddString(CPLSPrintf("%f", a_ullr[0]));
    argv.AddString(CPLSPrintf("%f", a_ullr[1]));
    argv.AddString(CPLSPrintf("%f", a_ullr[2]));
    argv.AddString(CPLSPrintf("%f", a_ullr[3]));
  }

  GDALTranslateOptions* psOptions = GDALTranslateOptionsNew(argv.List(), nullptr);

  auto hRet = GDALTranslate("", GDALDataset::ToHandle(poSrcDS),
                            psOptions, nullptr);

  poSrcDS->ReleaseRef();

  auto poDS = cpl::down_cast<VRTDataset*>(GDALDataset::FromHandle(hRet));

  if( poDS )
  {
    poDS->SetDescription(dsn[0]);
    poDS->SetWritable(false);

    //poDataset->GetGeoTransform( adfGeoTransform );
    GDALGetGeoTransform(poDS, adfGeoTransform );

    GDALClose(poDS);
  }
  NumericVector out(6);
  for (int i = 0; i < 6; i++) out[i] = adfGeoTransform[i];
  return out;
}
