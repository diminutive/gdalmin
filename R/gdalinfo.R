gdalmin_info_internal <- function(x, json = TRUE,
                         stats = FALSE,
                         checksum = FALSE,
                         wkt_format = "WKT2",
                         oo = character(),
                         initial_format = character(),
                         options = character()) {

  rep_zip <- function(x, y) {
    as.vector(t(cbind(rep(x, length(y)), y)))
  }
  extra <- c(if(json) "-json",
             if (stats) "-stats",
             if (checksum) "-checksum",
             if (nchar(wkt_format[1]) > 0) c("-wkt_format", wkt_format[1L]),
             if (length(oo) > 0 && any(nchar(oo) > 0) ) rep_zip("-oo", oo[nchar(oo) > 0]),
             if (length(initial_format) > 0 && any(nchar(initial_format) > 0)) rep_zip("-if", initial_format[nchar(initial_format) > 0]))

  options <- c(options, "-proj4", "-listmdd", extra)
  options <- options[!is.na(options)]  ## cant do unique because repeated arguments possible things like "-if" "GTiff" "-if" "netCDF"

  info <- gdal_info_cpp(x, options)
 info
}
