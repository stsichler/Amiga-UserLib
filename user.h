#ifndef USER_USER_H	/* User.lib Includefile für SAS/C ® SSi 1993/94 */
#define USER_USER_H

#if (sizeof(int)==2)
#error short integers
#endif

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* Definierte Konstanten ------------------------------------------------ */

#define _pi 3.14159265359

#define CVERR_OK  	0  	// Fehlerkonstanten für die real/fp-Routinen
// neg. Werte sind Fehler, pos. Werte Warnungen
#define CVERR_PREC	-10	// falsche Fließkommapräzision)
#define CVERR_NULL	-20	// der Konversionsroutine wurde ein 0-Pointer übergeben
#define CVERR_CUT 	10	// Stellen mußten abgeschnitten werden
#define CVERR_EXP 	20	// Exponent kann nur verfälscht wiedergegeben werden

/* Strukturen/Datentypen ------------------------------------------------ */

typedef struct	/* Datentyp für die Vektorroutinen s.u. */
{
	double x;
	double y;
	double z;
} vector;

/* Datentyp für die Timehandleroutinen (TIME = struct TimeHandle) */

typedef struct
{
	struct MsgPort ReplyPort;
	struct timerequest Request;
} TIME;

/* Datentyp für die Konversionsroutinen --------------------------------- */

typedef struct
{
	ULONG dat[3];
} real;

/* Datentyp für DynamicBlocks ------------------------------------------- */

typedef struct
{
	char *data;
	int datalen,maxlen,step;
} *dynamicblock;

/* Prototypen ----------------------------------------------------------- */

int cdcstr(char *,char *,char);		/* String-Funktionen */
int cdcnstr(char *,char *,char,int);
int cdrstr(char *,char *,char);
int cdrnstr(char *,char *,char,int);

#define getstr(s)  (s?strdup(s):0)	/* Speicherallok. für Strings - */
#define freestr(s) { if (s) free(s); }/* nur zur Kompatibilität */

extern char *strdup(const char *);	/* dummy */
extern void free(void *);				/* dummy */

int tstmouse(void);					/* Mouse-Test */
void waitmouse(void);

struct Node *FindNameType(struct List *,char *,UBYTE);	/* Listenverw. */

vector *VAdd(vector *,vector *,vector *);	/* v=v1+v2 */
vector *VSub(vector *,vector *,vector *); /* v=v1-v2 */
double   VSProd(vector *,vector *);		/* x=v1·v2 */
vector *VProd(vector *,vector *,vector *);/* v=v1×v2 */
double   VAngle(vector *,vector *);		/* x=v1,v2 */
double   VAngleN(vector *,vector *);		/* x=v1,v2 wenn v1,v2 normiert */
double   VLen(vector *);					/* x=|v| */
vector *VNorm(vector *);					/* v=v0 */
vector *VSMul(vector *,double);			/* v=v*x */
vector *VSDiv(vector *,double);			/* v=v/x */

TIME *OpenTime(void);
TIME *OpenTimeExactly(void);
TIME *OpenTimeInterrupt(void __asm (*code)(register __a1 void *),void *data);
void CloseTime(TIME *);
void StartTime(TIME *,ULONG);	/* Timerwert in Sekunden */
void StartTimeExactly(TIME *,ULONG,ULONG); /* Timerwert in secs/mics */
int CheckTime(TIME *);  		/* 0 - nicht abgel. / !=0 - abgelaufen */
void WaitTime(TIME *);
void StopTime(TIME *);

/* Environment Variablen ENV:/ENVARC: */

int PutEnv(char *name, APTR buffer, int length, BOOL save); /* ERROR= */
int GetEnv(char *name, APTR buffer, int length);	/* Bytes read= */

/* Konversionsroutinen (für floats/doubles) ------------------------------ */

#define toreal_std toreal_ieee			// nur _ffp und _ieee vorhanden !!!!
#define byreal_std byreal_ieee
#define toreal_881 toreal_ieee
#define byreal_881 byreal_ieee

#ifdef _FFP
#define toreal_specific toreal_ffp
#define byreal_specific byreal_ffp
#endif
#ifdef _IEEE
#define toreal_specific toreal_ieee
#define byreal_specific byreal_ieee
#endif
#ifdef _M68881
#define toreal_specific toreal_881
#define byreal_specific byreal_881
#endif
#ifndef toreal_specific
#define toreal_specific toreal_std
#define byreal_specific byreal_std
#endif

#define toreal(target,source) toreal_specific(target,source,sizeof(*(source)))
#define byreal(target,source) byreal_specific(target,source,sizeof(*(target)))

/* Schein-Prototypen:
 * int toreal(real *target,double *source); ERROR= (CVERR_...) (s.o.)
 * int byreal(double *target,real *source); ERROR= (CVERR_...) (s.o.)
 */

int toreal_std(real *,void *,int);
int byreal_std(void *,real *,int);
int toreal_ffp(real *,void *,int);
int byreal_ffp(void *,real *,int);
int toreal_ieee(real *,void *,int);
int byreal_ieee(void *,real *,int);
int toreal_881(real *,void *,int);
int byreal_881(void *,real *,int);

/* DynamicBlocks Routinen ------------------------------------------------- */

dynamicblock createblock(int step);
void destroyblock(dynamicblock block);
int putblock(dynamicblock block, APTR data, int datalen);	/* ERROR= */
void clearblock(dynamicblock block);
#define blockdata(block) ((block)->data)
#define blocklen(block) ((block)->datalen)


#endif
