#include <Rcpp.h>
using namespace Rcpp;

#include "gdal_version.h"
#include "gdal.h"
#include "cpl_string.h"
#include "cpl_multiproc.h"
//#include "commonutils.h"
#include "gdal_utils_priv.h"




/************************************************************************/
/*                         GDALInfoOptionsForBinary()                   */
/************************************************************************/

static GDALInfoOptionsForBinary *GDALInfoOptionsForBinaryNew(void)
{
  return static_cast<GDALInfoOptionsForBinary *>(
      CPLCalloc(1, sizeof(GDALInfoOptionsForBinary)));
}

/************************************************************************/
/*                       GDALInfoOptionsForBinaryFree()                 */
/************************************************************************/

static void GDALInfoOptionsForBinaryFree( GDALInfoOptionsForBinary* psOptionsForBinary )
{
  if( psOptionsForBinary )
  {
    CPLFree(psOptionsForBinary->pszFilename);
    CSLDestroy(psOptionsForBinary->papszOpenOptions);
    CSLDestroy(psOptionsForBinary->papszAllowInputDrivers);
    CPLFree(psOptionsForBinary);
  }
}

// [[Rcpp::export]]
CharacterVector gdal_info_cpp(CharacterVector dsn,

                              CharacterVector options) {

// just always have these, then add json/cat, min_max/mm/stats/approx_stats, checksum, wkt_format, oo, if
 // CharacterVector options = CharacterVector({"-proj4","-listmdd"});
  // new arg [-json] or just concat print output
  // new args (min_max): [-mm] [-stats | -approx_stats]
  // [-hist]
  // ignore [-nogcp]
  // ignore [-nomd]
  // ignore [-norat]
  // ignore [-noct]
  // ignore [-nofl]
  // new arg [-checksum]
  // always [-proj4]
  // always [-listmdd]
  // all [-mdd domain|`all`]*
  // new arg [-wkt_format WKT1|WKT2|...]  WKT1|WKT2|WKT2_2015|WKT2_2018  (this only in GDAL 3.0)
  // new arg (sds) [-sd subdataset]
  // new arg [-oo NAME=VALUE]*
  // new arg [-if format]*

  // CharacterVector rv(1);
  // rv[0] = "sst.tif";
  // char *ap[1] = {"sst.tif"};
  // // char aa[3][4] = {"foo", "bar", "baz"};
  // // char **pp = &ap[0];
  // char **argv =  &ap[0];  //(char **)malloc(7 * sizeof(char*));
  // //argv[1] = (char *)"sst.tif";
  // //argv = CSLAddString(argv, "-stdout");

  CPLStringList argv;
  // not this way
  //argv = CSLAddString(argv, "gdalinfo");
  //argv = CSLAddString(argv, "sst.tif");
  //argv = CSLAddString(argv, "-stdout");
  //GDALInfoOptionsForBinary* psOptionsForBinary = GDALInfoOptionsForBinaryNew();
  //GDALInfoOptions *psOptions = GDALInfoOptionsNew(argv+1, psOptionsForBinary);
  // this way, and (I think) don't destroy or free
  argv.AddString(dsn[0]);
  //argv.AddString("-stdout");   // internally gdalinfo applies this
  // if (json[0]) argv.AddString("-json");
  // if (stats[0]) argv.AddString("-stats");
  // if (checksum[0]) argv.AddString("-checksum");
  //
  // argv.AddString(CPLSPrintf("-wkt_format %s", wkt_format[0]));
  for (int i = 0; i < options.size(); i++) {
   argv.AddString(options[i]);
  }
  //
  GDALInfoOptionsForBinary* psOptionsForBinary = GDALInfoOptionsForBinaryNew();
  GDALInfoOptions *psOptions = GDALInfoOptionsNew(argv.List(), psOptionsForBinary);




   if( psOptions == nullptr ){
  //   //Usage();
  stop("eek\n");

   }
  //
   if( psOptionsForBinary->pszFilename == nullptr ) {
  //   //Usage("No datasource specified.");
     stop("eek again\n");
   }

  /* -------------------------------------------------------------------- */
  /*      Open dataset.                                                   */
  /* -------------------------------------------------------------------- */

  GDALDatasetH hDataset
    = GDALOpenEx( psOptionsForBinary->pszFilename,
                  GDAL_OF_READONLY | GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR,
                  psOptionsForBinary->papszAllowInputDrivers,
                  psOptionsForBinary->papszOpenOptions, nullptr );


   if( hDataset == nullptr )
   {
   stop("gdalinfo failed - unable to open '%s'.\n",
          psOptionsForBinary->pszFilename );
     //gdalinfo_bin.cpp has further logic here for vsitar/vsizip to list contents
   }

   char* pszGDALInfoOutput = GDALInfo( hDataset, psOptions );

   CharacterVector out(1);
   if( pszGDALInfoOutput ) {
    // printf( "%s", pszGDALInfoOutput );
    out[0] = pszGDALInfoOutput;
   } else {
     out[0] = NA_STRING;
   }
   CPLFree( pszGDALInfoOutput );


  GDALInfoOptionsForBinaryFree(psOptionsForBinary);
  //
  GDALInfoOptionsFree( psOptions );
  //
  GDALClose(hDataset);


  //GDALDumpOpenDatasets( stderr );

  //GDALDestroyDriverManager();

  //CPLDumpSharedList( nullptr );

  return out;
}
