#include "gdal_priv.h"
//#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include <iostream>

#include "cpl_string.h"
#include "cpl_error.h"
//#include "commonutils.h"
#include "gdal_version.h"
#include "gdal_utils_priv.h"

// #include "cpl_string.h"
// #include "cpl_error.h"
// #include "commonutils.h"
// #include "gdal_version.h"
//#include "gdal_utils_priv.h"
//#include "gdal_vrt.h"


/************************************************************************/
/*                       GDALBuildVRTOptionsForBinaryNew()              */
/************************************************************************/

static GDALBuildVRTOptionsForBinary *GDALBuildVRTOptionsForBinaryNew(void)
{
  return static_cast<GDALBuildVRTOptionsForBinary *>(
      CPLCalloc(1, sizeof(GDALBuildVRTOptionsForBinary)));
}

/************************************************************************/
/*                       GDALBuildVRTOptionsForBinaryFree()            */
/************************************************************************/

static void GDALBuildVRTOptionsForBinaryFree( GDALBuildVRTOptionsForBinary* psOptionsForBinary )
{
  if( psOptionsForBinary )
  {
    CSLDestroy(psOptionsForBinary->papszSrcFiles);
    CPLFree(psOptionsForBinary->pszDstFilename);
    CPLFree(psOptionsForBinary);
  }
}



// g++  -I/usr/include/gdal -o bvrt bvrt.cpp -L/usr/lib -lgdal -lproj
int main() {

GDALBuildVRTOptionsForBinary* psOptionsForBinary = GDALBuildVRTOptionsForBinaryNew();

CPLStringList afile;
afile.AddString("in.tif");
GDALBuildVRTOptions *psOptions = GDALBuildVRTOptionsNew(afile, psOptionsForBinary);



// if( psOptionsForBinary->pszDstFilename == nullptr )
// {
// }
//
// if( !(psOptionsForBinary->bQuiet) )
// {
//   GDALBuildVRTOptionsSetProgress(psOptions, GDALTermProgress, nullptr);
// }
//
// /* Avoid overwriting a non VRT dataset if the user did not put the */
// /* filenames in the right order */
// VSIStatBuf sBuf;
// if (!psOptionsForBinary->bOverwrite)
// {
//   int bExists = (VSIStat(psOptionsForBinary->pszDstFilename, &sBuf) == 0);
//   if (bExists)
//   {
//     GDALDriverH hDriver = GDALIdentifyDriver( psOptionsForBinary->pszDstFilename, nullptr );
//     if (hDriver && !(EQUAL(GDALGetDriverShortName(hDriver), "VRT") ||
//         (EQUAL(GDALGetDriverShortName(hDriver), "API_PROXY") &&
//         EQUAL(CPLGetExtension(psOptionsForBinary->pszDstFilename), "VRT"))) )
//     {
//       fprintf(stderr,
//               "'%s' is an existing GDAL dataset managed by %s driver.\n"
//               "There is an high chance you did not put filenames in the right order.\n"
//               "If you want to overwrite %s, add -overwrite option to the command line.\n\n",
//               psOptionsForBinary->pszDstFilename, GDALGetDriverShortName(hDriver), psOptionsForBinary->pszDstFilename);
//       Usage();
//     }
//   }
// }
//
// int bUsageError = FALSE;
// GDALDatasetH hOutDS = GDALBuildVRT(psOptionsForBinary->pszDstFilename,
//                                    psOptionsForBinary->nSrcFiles,
//                                    nullptr,
//                                    psOptionsForBinary->papszSrcFiles,
//                                    psOptions, &bUsageError);
// if( bUsageError )
//   Usage();
// int nRetCode = (hOutDS) ? 0 : 1;

GDALBuildVRTOptionsFree(psOptions);
GDALBuildVRTOptionsForBinaryFree(psOptionsForBinary);

return 1;
}
