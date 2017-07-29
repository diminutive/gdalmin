#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _gdalmin_ogr_api();

static const R_CallMethodDef CallEntries[] = {
    {"_gdalmin_ogr_api", (DL_FUNC) &_gdalmin_ogr_api, 0},
    {NULL, NULL, 0}
};

void R_init_gdalmin(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
