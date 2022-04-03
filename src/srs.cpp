#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"



// [[Rcpp::export]]
bool not_die1(CharacterVector proj) {
  OGRSpatialReference srs;
  srs.SetFromUserInput(proj[0]);
  char *txt = nullptr;
  srs.exportToProj4(&txt);
  if (txt == nullptr) {
    Rprintf("no result\n");
  } else {
    Rprintf("%s\n", txt);
  }
  CPLFree(txt);
  return true;
}

// [[Rcpp::export]]
bool do_die1(CharacterVector proj) {
  OGRSpatialReference *srs = nullptr;
  srs = new OGRSpatialReference;
  srs->SetFromUserInput("OGC:CRS84");
  char *txt = nullptr;
  srs->exportToProj4(&txt);
  delete srs;
  Rprintf("%s\n", txt);
  CPLFree(txt);
  return true;
}
