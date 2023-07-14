#ifndef USER_POINTERLISTS_H	/* User.lib Includefile für SAS/C ® SSi 1993-95 */
#define USER_POINTERLISTS_H

#if (sizeof(int)==2)
#error short integers
#endif

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* Datentyp für dir PointerList-Routinen (PL...) */

struct PointerList
{
	USHORT Allocated;
	USHORT Used;
	APTR Pointer[0];
};

typedef struct PointerList *PLIST;

extern USHORT __PLAllocStep;			/* Allokationsschrittweite der PL-Rout. */
APTR PLAddPos(PLIST *,USHORT,APTR);	/* fügt einen Zeiger hinzu */
APTR PLRemPos(PLIST *,USHORT);		/* entfernt einen Zeiger */
APTR PLRepPos(PLIST *,USHORT,APTR);	/* ersetzt einen Zeiger */
APTR PLGetPos(PLIST *,USHORT);		/* liest einen Zeiger aus */
USHORT PLGetNum(PLIST *);				/* übergibt die Anzahl der gesp. Zeiger */
void PLClear(PLIST *);				/* löscht und deallokiert die Liste */
USHORT PLPosOf(PLIST *,APTR);			/* sucht den Pointer in der Liste */
void PLCopyTo(PLIST *,PLIST *);		/* (to,from) kopiert die gesamte Liste */

#define PLAddHead(l,p) PLAddPos(l,0,p)
#define PLRemHead(l) PLRemPos(l,0)
#define PLRepHead(l,p) PLRepPos(l,0,p)
#define PLGetHead(l) PLGetPos(l,0)

#define PLAddTail(l,p) PLAddPos(l,PLGetNum(l),p)
#define PLRemTail(l) PLRemPos(l,PLGetNum(l)-1)
#define PLRepTail(l,p) PLRepPos(l,PLGetNum(l)-1,p)
#define PLGetTail(l) PLGetPos(l,PLGetNum(l)-1)

#endif
