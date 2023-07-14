/* FunctionRaw und FunctionCode Verwaltungsroutinen 26-12-94 ®SSi\94 */

#include <stdlib.h>
#include <user/functions.h>
#include <proto/exec.h>

/* FunctionRaw-Verwaltung ------------------------------------------------- */

fnctraw *allocfnctraw(size)	/* legt ein FnctRaw Feld an (size 512 meist */
unsigned short size;			/* ausreichend für alle Anwendungen) */
{							/* RETURN: Zeiger auf Feld, oder 0 bei Fehler */
	fnctraw *fraw;
	
	if (!(fraw=calloc(1,sizeof(fnctraw)))) return(0);
	fraw->size=size;
	if (
		(fraw->t=calloc(size,sizeof(rawtype))) &&
		(fraw->d=calloc(size,sizeof(rawdata))) &&
		(fraw->ds=calloc(size+2,sizeof(rawdata)))
		) return (fraw);

	freefnctraw(fraw);
	return(0);
}

void freefnctraw(raw)		/* gibt das Feld wieder frei (geschieht */
fnctraw *raw;				/* bei exit automatisch) */
{
	if (!raw) return;
	if (raw->t) free(raw->t);
	if (raw->d) free(raw->d);
	if (raw->ds) free(raw->ds);
	free(raw);
}

/* FunctionCode - Verwaltung --------------------------------------------- */

fnctcode *allocfnctcode(struct VarList *gvars,unsigned long *brkaddr)
/* legt ein FnctCode Feld an und übernimmt die geg. Vars *
 * brkaddr zeigt auf eine Adresse(!=0),dessen Inhaltsänderung den Abbruch einer
 * herbeiführt.
 */
{                   
	fnctcode *code=0;
	
	if (code=calloc(1,sizeof(fnctcode)))
	{
		code->global_varpool=gvars;
		NewList((struct List *)&code->varraws);
		code->brkaddr=brkaddr?brkaddr:(unsigned long *)&code->brkaddr;	
	}
	return(code);
}

void freefnctcode(fnctcode *code)		/* gibt das Feld wieder frei */
{
	if (code)
	{
		short a;
		fnctcode *cc=(fnctcode *)-1;
		struct fnctraw_entry *fre,*succ;
		
		DelAllVars(&code->local_varpool);
		freefnctraw(code->targetraw);
		
		for (fre=succ=(struct fnctraw_entry *)code->varraws.mlh_Head;succ->node.mln_Succ;fre=succ)
		{
			freefnctraw(fre->fraw);
			freefnctraw(fre->fraw_init);
			succ=(struct fnctraw_entry *)fre->node.mln_Succ;
			free(fre);
		}
		
		for (a=0;cc;a++)
			freefnctcode(cc=(fnctcode *)PLGetPos(&code->used_codes,a));
		
		free(code);
	}
}
