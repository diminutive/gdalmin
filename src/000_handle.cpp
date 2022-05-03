#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"

GDALDatasetH handle_cpp(const char * dsn) {
  GDALDatasetH DS;
  DS = GDALOpen(dsn, GA_ReadOnly);
  return DS;
}

GDALDataset * pointer_cpp(const char * dsn) {
  GDALDataset * poDS;
  poDS = (GDALDataset *)GDALOpen(dsn, GA_ReadOnly);
  return poDS;
}
// [[Rcpp::export]]
IntegerVector handle(const char * dsn) {
  GDALDatasetH x = handle_cpp(dsn);
  if (!(x == nullptr)) GDALClose(x);

  return IntegerVector::create(1);

}
// [[Rcpp::export]]
IntegerVector pointer(const char * dsn) {
  GDALDataset* x = pointer_cpp(dsn);
  if (!(x == nullptr)) GDALClose(x);

  return IntegerVector::create(1);

}

// [[Rcpp::export]]
IntegerVector handles(CharacterVector dsn) {
  GDALDatasetH* poHDS;
  // whoever calls this will have to CPLFree() this
  poHDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * dsn.size()));

  for (int i = 0; i < dsn.size(); i++) {
    poHDS[i] = handle_cpp(dsn[i]);
  }
  for (int i = 0; i < dsn.size(); i++) {
    if(!(poHDS[i] == nullptr)) GDALClose(poHDS[i]);

  }
  CPLFree(poHDS);
  return IntegerVector::create(1);

}
// [[Rcpp::export]]
IntegerVector pointers(CharacterVector dsn) {
  GDALDataset ** poDS;
  // // whoever calls this will have to CPLFree() this
  poDS = static_cast<GDALDataset **>(CPLMalloc(sizeof(GDALDataset *) * dsn.size()));

  for (int i = 0; i < dsn.size(); i++) {
    poDS[i] = pointer_cpp(dsn[i]);
  }
  for (int i = 0; i < dsn.size(); i++) {
    if(!(poDS[i] == nullptr)) GDALClose(poDS[i]);

  }
  CPLFree(poDS);
  return IntegerVector::create(1);

}
