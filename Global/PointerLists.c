/* PointerLists.c - Funktionen für einfache dynamische Zeigerlisten */

#include <user/user.h>
#include <user/pointerlists.h>
#include <stdlib.h>
#include <string.h>

/* Wird eine einfache Pointerliste in eine Struktur eingefügt, so
 * bindet man einfach einen Eintrag vom Typ PLIST ein.
 * PLIST ist ein Synonym für "struct PointerList *".
 * Allen PL-Routinen wird die Adresse(!) des PLIST-Eintrages übergeben.
 * Um eine leere Liste zu erzeugen, muß der PLIST-Eintrag null sein!
 * ACHTUNG: auto-Variablen sind nicht(!) bei Initialisierung null.
 *
 * Alle PL-Routinen verwenden malloc() und free(), alle Listen werden also
 * bei Programmende automatisch freigegeben.
 */

APTR PLAddPos(pl,pos,ptr) 			/* fügt einen Zeiger an der ange- */
PLIST *pl;							/* gebenen Position hinzu; ist die */
USHORT pos;							/* Position außerhalb der Liste, wird */
APTR ptr;								/* er ans Ende gehängt */
{									/* RETURN: der Zeiger selbst */
	if (!pl) return(0);
	if (!(*pl))
	{
		if (!(*pl=(PLIST)malloc(sizeof(struct PointerList)+__PLAllocStep*sizeof(APTR)))) return(0);
		(*pl)->Allocated=__PLAllocStep;
		(*pl)->Used=0;
	}
	if (pos>(*pl)->Used) pos=(*pl)->Used;
	(*pl)->Used+=1;
	
	/* Feld gegf. vergrößern */
	if (((*pl)->Used/__PLAllocStep+1)*__PLAllocStep!=(*pl)->Allocated)
	{
		PLIST newpl;
		int neededpointers=((*pl)->Used/__PLAllocStep+1)*__PLAllocStep;
		if (newpl=(PLIST)malloc(sizeof(struct PointerList)+neededpointers*sizeof(APTR)))
		{
			newpl->Allocated=neededpointers;
			newpl->Used=(*pl)->Used;
			memmove(&newpl->Pointer[0],&(*pl)->Pointer[0],(*pl)->Used*sizeof(APTR));
			
			free(*pl);
			*pl=newpl;
		}
		else
		{
			(*pl)->Used-=1;
			return(0);
		}
	}
	
	/* Zeiger ptr an Position pos einfügen: */
	memmove(&(*pl)->Pointer[pos+1],&(*pl)->Pointer[pos],((*pl)->Used-pos-1)*sizeof(APTR));	
	(*pl)->Pointer[pos]=ptr;
	return(ptr);
}

APTR PLRemPos(pl,pos)			/* entfernt einen Zeiger aus der Liste */
PLIST *pl;					/* RETURN: Wert des gelöschten Zeigers */
USHORT pos;
{
	APTR ptr;
	if (!pl) return(0);
	if (!(*pl)) return(0);
	if (pos>=(*pl)->Used) return(0);
	ptr=(*pl)->Pointer[pos];
	
	/* Zeiger an Position pos entfernen: */
	memmove(&(*pl)->Pointer[pos],&(*pl)->Pointer[pos+1],((*pl)->Used-pos-1)*sizeof(APTR));
	(*pl)->Used-=1;

	if ((*pl)->Used)
	{
		/* Feld gegf. verkleinern */
		if (((*pl)->Used/__PLAllocStep+1)*__PLAllocStep!=(*pl)->Allocated)
		{
			PLIST newpl;
			int neededpointers=((*pl)->Used/__PLAllocStep+1)*__PLAllocStep;
			if (newpl=(PLIST)malloc(sizeof(struct PointerList)+neededpointers*sizeof(APTR)))
			{
				newpl->Allocated=neededpointers;
				newpl->Used=(*pl)->Used;
				memmove(&newpl->Pointer[0],&(*pl)->Pointer[0],(*pl)->Used*sizeof(APTR));
				
				free(*pl);
				*pl=newpl;
			}
		}
	}
	else
	{
		free(*pl);
		*pl=0;
	}								
	return(ptr);
}

APTR PLRepPos(pl,pos,ptr)				/* ersetzt einen Zeiger */
PLIST *pl;							/* RETURN: alter Zeiger */
USHORT pos;
APTR ptr;
{
	APTR oldptr;
	if (!pl) return(0);
	if (!(*pl)) return(0);
	if (pos>=(*pl)->Used) return(0);
	
	oldptr=(*pl)->Pointer[pos];
	(*pl)->Pointer[pos]=ptr;
	return(oldptr);
}

APTR PLGetPos(pl,pos)						/* liest einen Zeiger aus */
PLIST *pl;								/* RETURN: Wert des Zeigers */
USHORT pos;
{
	if (!pl) return(0);
	if (!(*pl)) return(0);
	if (pos>=(*pl)->Used) return(0);
	
	return((*pl)->Pointer[pos]);
}

USHORT PLGetNum(pl)				/* übergibt die Anzahl der gesp. Zeiger */
PLIST *pl;
{
	if (!pl) return(0);
	if (!(*pl)) return(0);
	return((*pl)->Used);
}

void PLClear(pl)					/* löscht und deallokiert die Liste */
PLIST *pl;
{
	if (!pl) return;
	if (!(*pl)) return;
	free(*pl);
	*pl=0;
}

USHORT PLPosOf(pl,ptr)			/* sucht den Zeiger in der Liste und */
PLIST *pl;						/* übergibt die Nummer oder ~0 bei */
APTR ptr;							/* nicht enthalten */
{
	USHORT i;
	if (!pl) return((USHORT)~0);
	if (!(*pl)) return((USHORT)~0);

	for (i=0; i<PLGetNum(pl); i++) if ((*pl)->Pointer[i]==ptr) return(i);

	return((USHORT)~0);
}

void PLCopyTo(topl,frompl)		/* kopiert die komplette Liste */
PLIST *topl, *frompl;
{
	PLClear(topl);
	
	if (!(*topl=(PLIST)malloc(sizeof(struct PointerList)+(*frompl)->Allocated*sizeof(APTR)))) return;
	
	memcpy(*topl,*frompl,sizeof(struct PointerList)+(*frompl)->Allocated*sizeof(APTR));
	return;
}
