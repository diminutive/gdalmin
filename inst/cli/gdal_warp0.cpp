// compare this to gdal_warp.cpp, this is the version from the GDAL Warp API
// gdal_warp.cpp uses the GDALDaset * rather than GDALDatasetH

#include "cpl_conv.h"
#include "gdalwarper.h"
//g++  -I/usr/include/gdal -o gdal_warp0 gdal_warp0.cpp -L/usr/lib -lgdal -lproj
int main()
{
  GDALDatasetH  hSrcDS, hDstDS;

  // Open input and output files.
  GDALAllRegister();
  hSrcDS = GDALOpen( "in.tif", GA_ReadOnly );
  hDstDS = GDALOpen( "out.tif", GA_Update );

  // Setup warp options.
  GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->hSrcDS = hSrcDS;
  psWarpOptions->hDstDS = hDstDS;
  psWarpOptions->nBandCount = 1;
  psWarpOptions->panSrcBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panSrcBands[0] = 1;
  psWarpOptions->panDstBands =
    (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
  psWarpOptions->panDstBands[0] = 1;
  psWarpOptions->pfnProgress = GDALTermProgress;

  // Establish reprojection transformer.
  psWarpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer( hSrcDS,
                                     GDALGetProjectionRef(hSrcDS),
                                     hDstDS,
                                     GDALGetProjectionRef(hDstDS),
                                     FALSE, 0.0, 1 );
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Initialize and execute the warp operation.
  GDALWarpOperation oOperation;
  oOperation.Initialize( psWarpOptions );
  oOperation.ChunkAndWarpImage( 0, 0,
                                GDALGetRasterXSize( hDstDS ),
                                GDALGetRasterYSize( hDstDS ) );
  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( psWarpOptions );
  GDALClose( hDstDS );
  GDALClose( hSrcDS );
  return 0;
}
