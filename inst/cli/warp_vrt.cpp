
// compare this to gdal_warp.cpp, this is the version from the GDAL Warp API
// gdal_warp.cpp uses the GDALDaset * rather than GDALDatasetH

#include <iostream>
#include "cpl_conv.h"
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions
#include "gdal.h"
#include "vrtdataset.h"
//g++  -I/usr/include/gdal -o warp_vrt warp_vrt.cpp -L/usr/lib -lgdal -lproj
int main()
{

  GDALDatasetH *poSrcDS;
  GDALAllRegister();

  GDALDatasetH DS;
  int nfiles = 1;
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * nfiles));

  for (int i = 0; i < nfiles; i++) {
    DS = GDALOpen("in.tif", GA_ReadOnly);
    poSrcDS[i] = static_cast<GDALDatasetH *>(DS);
  }

  char** papszArg = nullptr;
  // https://github.com/OSGeo/gdal/blob/fec15b146f8a750c23c5e765cac12ed5fc9c2b85/gdal/frmts/gtiff/cogdriver.cpp#L512
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "VRT");

  papszArg = CSLAddString(papszArg, "-t_srs");
  papszArg = CSLAddString(papszArg, "+proj=laea");

  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", -3e6));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", -3e6));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", 3e6));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", 3e6));

  papszArg = CSLAddString(papszArg, "-ts");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", 512));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", 256));


  // hRet is a VRT dataset, with no file
  //papszArg = CSLAddString(papszArg, "VRT");
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  GDALDataset *hRet = (GDALDataset *)GDALWarp( "", nullptr,
                       nfiles, poSrcDS,
                       psOptions, nullptr);
  const char*xmlvrt = GDALGetMetadata(hRet, "xml:VRT")[0];
  std::cout << xmlvrt << "\n";

  GDALWarpAppOptionsFree(psOptions);
  for (int si = 0; si < nfiles; si++) {
    GDALClose( poSrcDS[si] );
  }
  CPLFree(poSrcDS);

  CPLAssert( hRet != NULL );
  GDALClose(hRet);
  std::cout << "we finished\n";
  return 1;
}
