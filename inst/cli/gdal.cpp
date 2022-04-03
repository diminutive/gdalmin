#include <gdal.h>
#include <ogr_spatialref.h>
#include <iostream>
  int main() {
    //GDALAllRegister();
    OGRSpatialReference srs;
    srs.SetFromUserInput((char *)"WGS84");
    char *txt = nullptr;
    srs.exportToProj4(&txt);
    std::cout << txt << "\n";
    CPLFree(txt);
  }

