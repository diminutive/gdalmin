<!-- README.md is generated from README.Rmd. Please edit that file -->
gdalmin
=======

This is a working minimal R package example of using the GDAL C++ API with Rcpp.

It was developed on Ubuntu, and has the following vagaries:

-   relies on install of libgdal-dev
-   relies on GNU autoconf being installed
-   copies the autoconf configure.ac from sf
-   copies the Makevars.in from sf

todo
----

-   document required system installs
-   use GNU automake/autoconf appropriately, not by copying artefact
-   work out cross-platform needs (rwinlib, brewer and so forth)

set up notes
------------

``` r
devtools::create("../gdalmin")
devtools::use_readme_rmd()
devtools::use_package_doc()
## useDynLib and importFrom Rccp in package doc
devtools::use_rcpp()

file.copy("inst/extdata/boilerplate/ogr_api_tutorial.cpp", "src/ogr_api.cpp", overwrite = TRUE)
## modify the C++ as below

## copy the contents of 
## https://raw.githubusercontent.com/dis-organization/vapour/master/configure.ac to configure.ac
```

Change the first line in 'configure.ac' to your C++ file, i.e.

    AC_INIT(src/ogr_api.cpp)

Modify src/ogr\_api.cpp to include Rcpp, roxygen doc comments, a non-void main C++ function, and a valid path to a shapefile (can have point or anything in it, but the example only prints out point summary, or just a message).

    #include <Rcpp.h>
    using namespace Rcpp;


    #include "ogrsf_frmts.h"
    //' OGR API
    //'
    //' OGR API tutorial minimal example
    //' @export
    //' @return integer
    //' @examples
    // [[Rcpp::export]]
    int ogr_api()
    {
      GDALAllRegister();
      GDALDataset       *poDS;
      poDS = (GDALDataset*) GDALOpenEx( "inst/extdata/shp/point.shp", GDAL_OF_VECTOR, NULL, NULL, NULL );
    ...


    Run the following

autoconf \#\# this will create configure from configure.ac \`\`\`

Now run the following, it creates 'src/init.c'.

``` r
tools::package_native_routine_registration_skeleton("../gdalmin", "src/init.c",character_only = FALSE)
```

Create src/Makevars.in

    PKG_CPPFLAGS=@PKG_CPPFLAGS@
    PKG_LIBS=@PKG_LIBS@
    CXX_STD=CXX11

Make sure roxygen is turned on for build and reload for this package. Now build the package.

Now run it!

``` r
library(gdalmin)
ogr_api()
#> [1] 0
```

Data raw
--------

``` r
library(sf)
pt <- st_sf(geometry = st_sfc(lapply(1:10, function(x) st_point(rnorm(2))), crs  = 3031), 
            a = 1:10, b = head(letters, 10))
st_write(pt, "inst/extdata/shp/point.shp")
```
