.onLoad <- function(libname, pkgname) {
  gdal_register()
}


#' gdalmin.
#'
#' @name gdalmin
#' @docType package
#' @useDynLib gdalmin
#' @importFrom Rcpp sourceCpp
NULL
