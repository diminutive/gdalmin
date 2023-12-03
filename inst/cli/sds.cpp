// g++ sds.cpp -I/usr/include/x86_64-linux-gnu  -I/usr/include -L/usr/lib/x86_64-linux-gnu -lgdal -lproj -o sds
// ./sds oisst-avhrr-v02r01.20220218.nc ice ## ok

#include "gdal.h"
#include "gdal_priv.h"
#include <iostream>
int main(int argc, char **argv) {
  GDALAllRegister();

  auto poSrcDS =
    GDALDataset::Open(argv[1], GDAL_OF_RASTER, nullptr, nullptr, nullptr);
  if (poSrcDS == nullptr)
  {
    return 0;
  }
  char **papszSubdatasets = poSrcDS->GetMetadata("SUBDATASETS");
  int nSubdatasets = CSLCount(papszSubdatasets);
  if (nSubdatasets > 0)
  {
    for (int j = 0; j < nSubdatasets; j += 2)
    {
      char* pszSubdatasetSource = CPLStrdup(strstr(papszSubdatasets[j], "=") + 1);
      GDALSubdatasetInfoH info = GDALGetSubdatasetInfo(pszSubdatasetSource);
      char* component = info ? GDALSubdatasetInfoGetSubdatasetComponent(info) : NULL;
      const bool bFound = component && EQUAL(argv[2], component);
      CPLFree(component);
      GDALDestroySubdatasetInfo(info);
      if ( bFound) {
        std::cout << pszSubdatasetSource << "\n";
        poSrcDS->ReleaseRef();
        poSrcDS = GDALDataset::Open(pszSubdatasetSource, GDAL_OF_RASTER, nullptr, nullptr, nullptr);
        CPLFree(pszSubdatasetSource);
        break;
      }
      else {
        CPLFree(pszSubdatasetSource);
      }
    }
  }

  poSrcDS->ReleaseRef();
  return 1;
}
