#include <iostream>
#include "gdal.h"
#include "ogr_spatialref.h"

bool this_way(char *input) {
  OGRSpatialReference srs;
  srs.SetFromUserInput(input);
  char *txt = nullptr;
  srs.exportToProj4(&txt);
  std::cout << txt << "\n";
  CPLFree(txt);
  return true;
}

bool cpl_way(CPLString input) {
  OGRSpatialReference srs;
  srs.SetFromUserInput(input);

  char *output = nullptr;
  srs.exportToProj4(&output);
  std::cout << output << "\n";
  CPLFree(output);
  return true;
}


bool that_way(char *input) {
  OGRSpatialReference *srs = nullptr;
  srs = new OGRSpatialReference;
  srs->SetFromUserInput(input);
  char *txt = nullptr;
  srs->exportToProj4(&txt);
  delete srs;
  std::cout << txt << "\n";

  CPLFree(txt);
  return true;
}

int main() {
  char input[] = "WGS84";
  bool yes = this_way(input);
  bool no  = that_way(input);
  CPLString   input2{};
  input2 = "WGS84";
  bool cpl = cpl_way(input2);
  return 1;
}
