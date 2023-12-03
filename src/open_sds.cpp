#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
#include "gdal.h"
// [[Rcpp::export]]
NumericVector open_sds(CharacterVector dsn, CharacterVector sds) {

  GDALAllRegister();
  auto poSrcDS = GDALDataset::Open(dsn[0], GDAL_OF_RASTER, nullptr, nullptr, nullptr);
  if (poSrcDS == nullptr)
  {
    return NumericVector::create( 0.0);
  }

  char **papszSubdatasets = GDALGetMetadata(poSrcDS, "SUBDATASETS");
  // note this is 2x the actual subdatasets, we skip the odd numbers and check 0,3,5,7, the NAME= line
  // e.g.
  //SUBDATASET_1_NAME=NETCDF:"20220102090000-JPL-L4_GHRSST-SSTfnd-MUR25-GLOB-v02.0-fv04.2.nc":analysed_sst
  //SUBDATASET_1_DESC=[1x720x1440] sea_surface_foundation_temperature (16-bit integer)
  int nSubdatasets = CSLCount(papszSubdatasets);
  Rprintf("%i\n", nSubdatasets);
  if (nSubdatasets > 0)
  {
     for (int j = 0; j < nSubdatasets; j += 2)
     {
     char* pszSubdatasetSource = CPLStrdup(strstr(papszSubdatasets[j], "=") + 1);
     GDALSubdatasetInfoH info = GDALGetSubdatasetInfo(pszSubdatasetSource);
     char* component = info ? GDALSubdatasetInfoGetSubdatasetComponent(info) : NULL;
     Rprintf("%s\n", component);
     const bool bFound = component && EQUAL(sds[0], component);
      CPLFree(component);
      GDALDestroySubdatasetInfo(info);
      if ( bFound) {
        std::cout << pszSubdatasetSource << "\n";
        poSrcDS->ReleaseRef();
        Rprintf("%s\n", pszSubdatasetSource);
        poSrcDS = GDALDataset::Open(pszSubdatasetSource, GDAL_OF_RASTER, nullptr, nullptr, nullptr);
        CPLFree(pszSubdatasetSource);
        break;
      }
      else {
        CPLFree(pszSubdatasetSource);
      }

     }
   }

  poSrcDS->ReleaseRef();
  return NumericVector::create(1.0);
}

