#include <Rcpp.h>
using namespace Rcpp;
#include "gdal_priv.h"
#include "ogrsf_frmts.h"


void check_alive(SEXP ptr) {
  if( R_ExternalPtrAddr(ptr) == NULL )
  {
    Rcpp::stop("external pointer is null\n");
  }

}

// [[Rcpp::export]]
SEXP gm_GDALOpenShared(CharacterVector dsn) {

  GDALAllRegister();
  // create pointer to an GDAL object and
  // wrap it as an external pointer
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenShared(dsn[0], GA_ReadOnly);

  if( poDS == NULL )
  {
    Rprintf("Problem with 'dsn' input: %s\n", dsn[0]);
    Rcpp::stop("Open failed.\n");
  }
  Rcpp::XPtr<GDALDataset> ptr(poDS);
  return ptr;
}

IntegerVector gm_GDALClose(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);
  GDALClose(poDS);

  return 0;
}
// [[Rcpp::export]]
CharacterVector gm_GetDescription(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->GetDescription();
}

// [[Rcpp::export]]
CharacterVector gm_chr(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->GetDescription();
}

// [[Rcpp::export]]
IntegerVector gm_GetRasterXSize(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);
  return Rcpp::wrap(poDS->GetRasterXSize());
}
// [[Rcpp::export]]
IntegerVector gm_GetRasterYSize(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  IntegerVector out(1);
  out[0] = poDS->GetRasterYSize();
  return out;
}
// [[Rcpp::export]]
IntegerVector gm_GetRasterCount(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);
  IntegerVector out(1);
  out[0] = poDS->GetRasterCount();
  return out;
}
// [[Rcpp::export]]
IntegerVector gm_GetGCPCount(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->GetGCPCount();
}
// [[Rcpp::export]]
IntegerVector gm_Reference(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->Reference();
}
// [[Rcpp::export]]
IntegerVector gm_Dereference(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->Dereference();
}
// [[Rcpp::export]]
IntegerVector gm_ReleaseRef(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->ReleaseRef();
}
// [[Rcpp::export]]
IntegerVector gm_GetRefCount(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return IntegerVector::create( poDS->GetRefCount());
}
// [[Rcpp::export]]
IntegerVector gm_GetSummaryRefCount(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->GetSummaryRefCount();
}
// [[Rcpp::export]]
IntegerVector gm_GetShared(SEXP xp) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);

  return poDS->GetShared();
}
// [[Rcpp::export]]
IntegerVector gm_TestCapability(SEXP xp, CharacterVector capability) {
  Rcpp::XPtr<GDALDataset> poDS(xp);
  check_alive(poDS);
  return poDS->TestCapability(capability[0]);
}
