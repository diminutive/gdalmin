#include "cpl_conv.h"
#include "gdal_priv.h"
#include "gdalwarper.h"
//g++  -I/usr/include/gdal -o gdal_warp0 gdal_warp0.cpp -L/usr/lib -lgdal -lproj
int main()
{
  GDALDataset  *hSrcDS;
  GDALDataset  *hDstDS;

  // Open input and output files.
  GDALAllRegister();
  hSrcDS = (GDALDataset *)GDALOpen( "in.tif", GA_ReadOnly );
  hDstDS = (GDALDataset *)GDALOpen( "out.tif", GA_Update );

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
    GDALCreateGenImgProjTransformer( GDALDataset::ToHandle(hSrcDS),
                                     hSrcDS->GetProjectionRef(),
                                     GDALDataset::ToHandle(hDstDS),
                                     hDstDS->GetProjectionRef(),
                                     FALSE, 0.0, 1 );
  psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Initialize and execute the warp operation.
  GDALWarpOperation oOperation;
  oOperation.Initialize( psWarpOptions );
  oOperation.ChunkAndWarpImage( 0, 0,
                                hDstDS->GetRasterXSize(),
                                hDstDS->GetRasterYSize() );
  GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
  GDALDestroyWarpOptions( psWarpOptions );
  GDALClose( hDstDS );
  GDALClose( hSrcDS );
  return 0;
}
