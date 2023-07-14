/* Includefile für Funktionen der user.lib in Functions.c,FnctVars.c, */
/* FnctRaw.c -------------------------------------------  2-4-94 ®SSi */

#ifndef USER_FUNCTIONS_H
#define USER_FUNCTIONS_H

#if (sizeof(int)==2)
#error short integers
#endif

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef USER_POINTERLISTS_H
#include <user/pointerlists.h>
#endif

/* für StringToRaw ----------------------------------------------------- */

#define RT_OP 1			/* alle normalen Rechenoperationen */
#define RT_VAL 2		/* feste Werte */
#define RT_VAR 3		/* Variablen */
#define RT_FUNC 4		/* (, - (als Negation) und Fkt. */
#define RT_APP 5		/* ! ² ³ */
#define RT_CODE 6		/* ^fnctcode (Benutzung entspr. RT_VAL) */ 

typedef struct
{
	double re,im;
} complex;

typedef union
{
	struct						/* (RawType =) */
	{
		short op;	/* Operator (Zeichen) (RT_OP) */
		short pri;	/* Operator (Priorität) */
	} op;
	complex val;		/* Float - Wert       (RT_VAL) */
	complex *var;		/* Variable (Zeiger auf Wert) (RT_VAR) */
	void *code;		/* Code der iterate()-Fnkt. (Zeiger auf fnctcode) (RT_CODE) */
} rawdata;

typedef char rawtype;

typedef struct	/* Grundstruktur zur Funktionszwischenspeicherung */
{
	unsigned short size;	/* Größe der Felder */ 
	rawtype *t;						/* Speicherfelder */
	rawdata	*d;
	rawdata *ds;					/* Stack */
} fnctraw;

/* für die Variablenroutinen ------------------------------------------ */

struct Var
{
	complex value;
	char name[32];
	struct Var *succ;
	struct Var *pred;
	short type;			/* s.u. VAR_ */
};

/* -------------------------------------------------------------------- */

struct VarList					/* VARIABLEN-WURZELSTRUKTUR */
{
	struct Var *first;
	struct Var *last;
};

/* Variablentypen (Var.type) ------------------------------------------ */

#define VAR_UNKNOWN -1/* unbekannt */
#define VAR_USER 1	/* vom Benutzer verwendet */
#define VAR_SYS 2		/* vom System */

/* Neue Funktionsstrukturen *********************************************/

struct fnctraw_entry		/* Eintrag im FnctRaw Array (MinList strukt.) s.u. */
{
	struct MinNode node;
	struct Var *var;		/* Variable, die hiermit def. wird */
	fnctraw *fraw;				/* (normaler) Iterationsterm */
	fnctraw *fraw_init;	/* Initialisierungsterm */
};

typedef struct MinList fnctraw_array;

struct _fnctcode	/* Zentralstruktur für einen Term ------------------ */
{
	struct _fnctcode *parent;		/* übergeordneter Term */
	short flags;								/* (FCF_...) */
	
	struct VarList *global_varpool;		/* globales Variablenfeld */
	struct VarList local_varpool;			/* lokales       -"-      */
	
	fnctraw *targetraw;					/* Zielterm */
	fnctraw_array varraws;			/* variablendef. Terme */
	
	struct Var *brkvar;					/* "brk"-Var. bei Iteration */
															/* (null, wenn keine Iteration) */
	unsigned long *brkaddr;			/* Abbruch-Adresse */
	PLIST used_codes;						/* weitere benutzte Code-Felder */
};

typedef struct _fnctcode fnctcode;

#define FCF_ITERATIVE (1<<0)		/* iteratives Codefeld */

/************************************************************************/

/* Operatorprioritäten ------------------------------------------------ */

/*		Fkt()       60
 *    ! ²	³ %	°   50		(%: 50% = .5 ) ( 180° = ¶)
 *		^           40
 *		*  :  /     30
 *		+  -  n	Fkt 20		(n als Negation)
 *		\           16    (\ als Boolsche Negation)
 *		< <= > >=   15    (<= = k / >= = g)
 *    = \=        14    (= als Boolscher Vergleich) (\= = u)
 *    &           13    (Boolsches Und)
 *    |           12    (Boolsches Oder)
 *		()          10
 *		=            0
 *
 */

/* Prototypen --------------------------------------------------------- */

short strtofnctraw(char[],fnctraw *,struct VarList *,short);
int evalfnctraw(complex *, fnctraw *);	/* ERROR= */

/* Neue Funktionsauswertungsroutinen **********************************/

short makefnctcode(char[],fnctcode *);
int evalfnctcode(complex *, fnctcode *);	/* ERROR= */

fnctcode *allocfnctcode(struct VarList *,unsigned long *);
void freefnctcode(fnctcode *);

/**********************************************************************/

struct Var *GetVariable(struct VarList *,char[],short);
void SetVariable(struct Var *,double,double);
void DelVariable(struct VarList *,struct Var *);
void DelAllVars(struct VarList *);

fnctraw *allocfnctraw(unsigned short);
void freefnctraw(fnctraw *);

/* von evalfnctraw() und evalfnctcode() erzeugte Fehler in _FPERR ---------- */

#define _FPEnfr	100		/* no function raw */
#define _FPEbrk	101		/* iteration brk */
#define _FPEnfv	102		/* negative or imaginary value to x! operation */
//#define _FPEnpv	103		/* negative value to ^ operation */
#define _FPEudf 103		/* undefined */
#define _FPEubp	104		/* unbalanced parantheses */
#define _FPEout	105		/* value out of function [asin/acos] range */
//#define _FPEnrv	106		/* negative value to sqrt() */
#define _FPEovf 106		/* overflow */
#define _FPEzlv	107		/* zero passed to ln() */
#define _FPEukn	108		/* unknown operator */


/* ------------------------------------------------------------------------- */

#endif
