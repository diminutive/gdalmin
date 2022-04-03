#include <iostream>
#include "gdal.h"
#include "cpl_port.h"

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions

// g++  -I/usr/include/gdal -L/usr/lib -lgdal -lproj -o warp warp.cpp
int warp() {
  GDALAllRegister();

  char input[] = "OGC:CRS84";
  const char* file[] = {"/rdsi/PUBLIC/raad/data/hs.pangaea.de/Maps/bathy/IBCSO_v1/ibcso_v1_is.tif",
                 "/rdsi/PRIVATE/raad/data_local/aad.gov.au/gebco/GEBCO_2019.tif"
                 };
  char outfile[] = "warp.tif";

  // hack the extents of the 2nd
  CPLStringList argv;
  argv.AddString("-of");
  argv.AddString("VRT");
  argv.AddString("-a_ullr");
  argv.AddString(CPLSPrintf("%f", -180.0));
  argv.AddString(CPLSPrintf("%f", 90.0));
  argv.AddString(CPLSPrintf("%f", 180.0));
  argv.AddString(CPLSPrintf("%f", -90.0));
  GDALTranslateOptions* psTransOptions = GDALTranslateOptionsNew(argv.List(), nullptr);


  GDALDatasetH DS = GDALOpen(file[1], GA_ReadOnly);
  GDALDatasetH h1 = GDALTranslate("", DS,
                                  psTransOptions, nullptr);
  GDALTranslateOptionsFree( psTransOptions );

  GDALDatasetH *ds;
  ds = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * 2));
  if (ds == nullptr) {
    std::cout << "stop\n";
    CPLFree(ds);
    return 0;
  }
  ds[0] = GDALOpen(file[0], GA_ReadOnly);
  ds[1] = static_cast<GDALDatasetH *>(h1);

  char** papszArg = nullptr;

  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "GTiff");
  papszArg = CSLAddString(papszArg, "-t_srs");
  papszArg = CSLAddString(papszArg, input);
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", -180.0));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", -90.0));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", 180.0));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", 90.0));
  papszArg = CSLAddString(papszArg, "-ts");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", 360));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", 180));


  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);


  auto hRet = GDALWarp( outfile, nullptr,
                        2, ds,
                        psOptions, nullptr);
  GDALClose(ds[0]);
  GDALClose(ds[1]);
  GDALClose(DS); // don't close this before the Warp
  //GDALClose(DS);
  //GDALClose(h1); // don't close this one

  GDALWarpAppOptionsFree(psOptions);
  CPLFree(ds);
//  GDALClose(ds);
  GDALClose(hRet);
  return 1;
}

int main() {

  int warp_test = warp();

  return 1;
}
