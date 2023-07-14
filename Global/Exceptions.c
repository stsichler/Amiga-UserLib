/* User.lib ®SSi 15.6.98 - Exception.c --------------------------------- */

#include <user/exceptions.h>
#include <stdlib.h>
#include <stdio.h>

static jmp_buf      _exception_env_stack[_EXCEPTION_MAX_NEST];
static unsigned int _exception_stackpos = 0;

static void _exception_error(int exc)
{
	fprintf(stderr,"\n *** FATAL ERROR: Uncaught exception no. %ld.\n\n", exc);
	exit(20);
}

jmp_buf *_exception_try(void)
{
	if (_exception_stackpos<_EXCEPTION_MAX_NEST) return &_exception_env_stack[_exception_stackpos++];
	else throw(EXCEP_STACK_OVERFLOW);
}

void _exception_throw(int exc)
{
	if (_exception_stackpos>0) longjmp(_exception_env_stack[--_exception_stackpos],exc);
	else _exception_error(exc);
}

int _exception_catch(int exc,int id)
{
	if (!exc || (exc && exc==id)) return 1;
	else return 0;
}

void _exception_endtry(int exc)
{
	if (!exc) --_exception_stackpos;
}
