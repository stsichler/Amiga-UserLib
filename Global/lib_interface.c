void __regargs __autoopenfail(char *);

#include <constructor.h>
#include <proto/exec.h> 

/* Library-abhängige Definitionen---------- */

#include <proto/interface.h>
#define LibraryName INTERFACENAME
#define LibraryVersion 0

#define BaseKind void *
#define BaseName InterfaceBase

#define OPENCONSTRUCT openinterface
#define CLOSECONSTRUCT closeinterface

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
