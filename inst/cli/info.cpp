// g++ info.cpp -I/usr/include/x86_64-linux-gnu  -I/usr/include -L/usr/lib/x86_64-linux-gnu -lgdal -lproj -o info
// ./info NETCDF:abc.nc:array1 NETCDF:abc.nc:array2 NETCDF:abc.nc:array3

#include "gdal.h"
#include "gdal_priv.h"
#include <iostream>
int main(int argc, char **argv) {
  GDALAllRegister();
  //GDALSubdatasetInfoH info = nullptr;
  std::cout << argc << "\n";
  if (argc > 1)
  {
    for (int j = 1; j < argc; j ++)
    {
      std::cout << argv[j] << "\n";
      GDALSubdatasetInfoH info = GDALGetSubdatasetInfo(argv[j]);

      if (info == nullptr) {
        //return 0;
      } else {
      std::cout << GDALSubdatasetInfoGetSubdatasetComponent(info) << "\n";
      GDALDestroySubdatasetInfo(info);
      }

    }
  }


  return 1;
}
