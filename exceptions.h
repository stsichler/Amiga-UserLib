#ifndef USER_EXCEPTIONS_H	/* User.lib Includefile für SAS/C ® SSi 1998 */
#define USER_EXCEPTIONS_H

#include <setjmp.h>

/* Nestcounter ------------------------------ */

/* max. nesting of try()-blocks */

#define _EXCEPTION_MAX_NEST 32

/* Predefined exceptions -------------------- */

#define EXCEPTION_NONE 0
#define EXCEPTION_others 0 
#define EXCEPTION_EXCEP_STACK_OVERFLOW -1

/* ------------------------------------------

 * An example code fragment *

#define EXCEPTION_MEMORY 1

	...
	
	try              * try...endtry; can be treated like a signle command block
	{
		pointer=malloc(bigsize);
		if (!pointer) throw(MEMORY);
		printf("pointer is %p!\n",pointer);
	}
	catch(MEMORY)    * There can be multiple catch()-blocks.
	{
		printf("Memory exception caught!\n");
	}
	catch(others)    * Catches all that has not been catched so far, if it is missing
	{                * then the default 'catch(others) passthru();' is assumed.
		printf("Unexpected exception! Passing through...\n");
		passthru();
	}
	endtry;
	
	...

   ------------------------------------------ */

#define try           { int _exception_id; if (!(_exception_id=setjmp(*_exception_try()))) 
#define endtry        else _exception_throw(_exception_id); _exception_endtry(_exception_id); } 
#define catch(e)      else if (_exception_catch(EXCEPTION_##e,_exception_id)) 
#define throw(e)      _exception_throw(EXCEPTION_##e) 
#define passthru()    _exception_throw(_exception_id) 
#define passthrough() _exception_throw(_exception_id) 

extern jmp_buf *_exception_try(void);
extern void _exception_throw(int exc);
extern void _exception_endtry(int);
extern int _exception_catch(int exc,int id);

#endif
