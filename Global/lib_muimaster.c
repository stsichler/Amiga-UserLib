void __regargs __autoopenfail(char *);

#include <constructor.h>
#include <proto/exec.h> 

/* Library-abhängige Definitionen---------- */

//#include <proto/muimaster.h>
#define LibraryName "muimaster.library"
#define LibraryVersion 0

#define BaseKind struct Library *
#define BaseName MUIMasterBase

#define OPENCONSTRUCT openmuimaster
#define CLOSECONSTRUCT closemuimaster

/* ---------------------------------------- */

BaseKind BaseName;
static BaseKind libbase;

CBMLIB_CONSTRUCTOR(OPENCONSTRUCT)
{
   libbase= BaseName= (BaseKind)OpenLibrary(LibraryName, LibraryVersion);
   if (BaseName == NULL) { __autoopenfail(LibraryName); return 1; }
   return 0;
}

CBMLIB_DESTRUCTOR(CLOSECONSTRUCT) { if (libbase) { CloseLibrary(libbase); libbase= BaseName= NULL; } }
