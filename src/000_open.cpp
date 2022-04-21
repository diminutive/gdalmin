#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>



// topos <- c("/vsicurl/https://public.services.aad.gov.au/datasets/science/GEBCO_2019_GEOTIFF/GEBCO_2019.tif",
//            "/vsicurl/https://opentopography.s3.sdsc.edu/raster/NASADEM/NASADEM_be.vrt")
//
//   file <-"../vapour/inst/extdata/volcano.tif"
// gdalmin:::open_gdal(file)
//   gdalmin:::open_gdal(c(file, topos))
//

// [[Rcpp::export]]
IntegerVector open_gdal(CharacterVector source_filename) {

  NumericVector source_extent(4);
  source_extent[0] = 0;
  source_extent[1] = 1;
  source_extent[2] = 0;
  source_extent[3] = 2;
  CharacterVector source_WKT(1);
  source_WKT[0] = "OGC:CRS84";

  GDALAllRegister();
  CPLStringList translate_argv;
  translate_argv.AddString("-of");
  translate_argv.AddString("VRT");


  bool set_extent = false;
  bool set_projection = false;

  if (set_extent) {
    if ((source_extent[1] <= source_extent[0]) || source_extent[3] <= source_extent[2]) {
      Rprintf("source_extent must be valid c(xmin, xmax, ymin, ymax)\n");
      stop("error: gdal_warp_in_memory");
    }
    translate_argv.AddString("-a_ullr");
    translate_argv.AddString(CPLSPrintf("%f", source_extent[0]));
    translate_argv.AddString(CPLSPrintf("%f", source_extent[3]));
    translate_argv.AddString(CPLSPrintf("%f", source_extent[1]));
    translate_argv.AddString(CPLSPrintf("%f", source_extent[2]));
  }
  if (set_projection) {
    translate_argv.AddString("-a_srs");
    translate_argv.AddString(source_WKT[0]);
  }

  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(translate_argv.List(), nullptr);

  GDALDatasetH *poSrcDS;
  GDALDataset *DS;

  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * source_filename.size()));

  //CharacterVector source_vrt(source_filename.size());

  for (int i = 0; i < source_filename.size(); i++) {
    DS = (GDALDataset *) GDALOpenShared(source_filename[i], GA_ReadOnly);
    // unwind everything, and stop
    if (DS == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose( poSrcDS[j]);
      }
      GDALTranslateOptionsFree( psTransOptions );
      CPLFree(poSrcDS);
      Rprintf("input source not readable: %s\n", (char *)source_filename[i]);
      Rcpp::stop("");
    }
    if (set_extent || set_projection) {
      // GDALDataset *h1;
      // h1 = (GDALDataset *)GDALTranslate("", DS, psTransOptions, nullptr);

      poSrcDS[i] = GDALTranslate("", DS, psTransOptions, nullptr);
      GDALClose((GDALDatasetH) DS);
    } else {
      poSrcDS[i] = (GDALDatasetH) DS;
    }
  }
  GDALTranslateOptionsFree( psTransOptions );

  for (int i = 0; i < source_filename.size(); i++) GDALClose((GDALDatasetH) poSrcDS[i]);
  CPLFree(poSrcDS);
  return 1;
}
