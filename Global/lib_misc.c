/* Autointroutine für die Misc.resource */

extern void __regargs __autoopenfail(char *);

#include <constructor.h>
#include <proto/exec.h> 

void *MiscBase;

CBMLIB_CONSTRUCTOR(openmisc)
{
   MiscBase = (void *)OpenResource("misc.resource");
   if (MiscBase == 0) { __autoopenfail("misc.resource"); return 1; }
   return 0;
}
