
#include <user/functions.h>
#include <proto/exec.h>
#include <exec/memory.h>
#include <string.h>

/* Variablen-Routinen ----------------------------------------------------- */

struct Var *GetVariable(vlist,name,type)
struct VarList *vlist;
char name[];
short type;
{
	struct Var *v;			/* bestehende Variable holen oder eine neue */
							/* anlegen und den Typ  eintragen */
	if (!vlist) return(0);
	
	v=vlist->first;
	
	while(v)
	{
		if (!strnicmp(v->name,name,31)) return(v);
		v=v->succ;
	}
	if (!type) return(0);
	
	if (!(v=AllocMem(sizeof(struct Var),MEMF_CLEAR))) return(0);
	
	strncpy(v->name,name,31);
	if (v->pred=vlist->last) v->pred->succ=v;
	else vlist->first=v;
	vlist->last=v;
	v->type=type;
	return(v);
}

void SetVariable(v,re,im)	/* eine bestehende Variable auf einen Wert setzen */
struct Var *v;
double re,im;
{
	if (v)
	{
		v->value.re=re;
		v->value.im=im;
	}
}

void DelVariable(vlist,v)			/* eine Variable löschen */ 
struct VarList *vlist;
struct Var *v;
{
	if (v->pred) v->pred->succ=v->succ;
	else vlist->first=v->succ;
	if (v->succ) v->succ->pred=v->pred;
	else vlist->last=v->pred;
	
	FreeMem(v,sizeof(struct Var));
}

void DelAllVars(vlist)		/* alle Variablen löschen */
struct VarList *vlist;
{
	while (vlist->first) DelVariable(vlist,vlist->first);
}
