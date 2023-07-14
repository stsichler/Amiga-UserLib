/* Plot - 25-12-93 SSi - Funktionsauswertung // user.lib 21-6-94 ®SSi/94 */

#include <proto/exec.h>
#include <exec/memory.h>
#include <error.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <user/functions.h>

#define _pi 3.14159265359

/* Operatorprioritäten ------------------------------------------------ */

/*		Fkt()       60
 *    ! ²	³ %	°   50		(%: 50% = .5 ) ( 180° = ¶)
 *		^           40
 *		*  :  /     30
 *		+  -  n	Fkt 20		(n als Negation)
 *		\  ?        16    (\ als Boolsche Negation)
 *		< <= > >=   15    (<= = k / >= = g)
 *    = \=        14    (= als Boolscher Vergleich) (\= = u)
 *    &           13    (Boolsches Und)
 *    |           12    (Boolsches Oder)
 *		()          10
 *		=            0
 *
 */

/* ------------------------------------------------------------------------- */

short strtofnctraw(Str,Raw,VList,VEnable)	
/* StringToFunctionRaw */
char Str[];            /* wandelt den String ins Raw-Format um */
fnctraw *Raw;          /* Zeiger auf das zu produzierende Raw-Feld */
struct VarList *VList; /* bei 0 - keine Variablen benutzen */
short VEnable;         /* wenn 0 - keine Vars anlegen / sonst anlegen erlaubt */
/* RETURN: 0 - ok. / !=0 Nummer des fehlerhaften */
/* Zeichens +1 !!!! */
{
	short a,b,c,r;	/* oder -1 bei allgem. Fehler */
	short pnest=0;
	char Buf[64];
 
	/* Funktionen ------ */

	static char *Function[] =
	{
		"sin","cos","tan","asin","acos","atan","abs",
		"cot","sinh","cosh","tanh","sqrt","ln","log",
		"exp","sin²","cos²","tan²","sgn",0
	};
	
	a=r=0;
	
	while (Str[a])
	{
		if (Str[a]==' ') { a++; continue; }
		
		if (pnest<0) return(a);	/* mehr Klammern zu als offen ? */
		
		if (strchr("01234567890.¶¼½¾",Str[a]))
		{
			/* Explizite Zahl ----------------------------------- */
								/* gegf. * einfügen */
			if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
			{
				Raw->t[r]=RT_OP;
				Raw->d[r].op.op='*'; Raw->d[r].op.pri=30;
				r++;
			}
			Raw->t[r]=RT_VAL;

			Raw->d[r].val.im=0.0;
			
			switch (Str[a])	/* Zahl übernehmen */
			{
				case '¶': Raw->d[r++].val.re=_pi;
						  a++; break;
				case '¼': Raw->d[r++].val.re=.25;				/* =.25 */
						  a++; break;
				case '½': Raw->d[r++].val.re=.5;				/* =.5 */
						  a++; break;
				case '¾': Raw->d[r++].val.re=.75;				/* =.75 */
						  a++; break;
				
			    default:	/* triviale Fließkommazahl übernehmen */
				{				
					short EMode=0;
					short Exp;		/* Charnum of Exp in Buf */
					short ExpStr;	/*       -"-      in Str */
					b=0;
					while (Str[a]>='0' && Str[a]<='9' || 
							(Str[a]=='.' || Str[a]=='E') && !EMode || 
							(Str[a]=='+' || Str[a]=='-') && EMode==1)
					{
						if (EMode==1) { Exp=b; ExpStr=a; }
						if (EMode) EMode++;
						if (Str[a]=='E') EMode++;
						Buf[b++]=Str[a++];
					}
					Buf[b]=0;
#ifdef _FFP
					/* Test, ob Fließkommazahl im erlaubten Bereich */
					if (EMode) 
					{
						double e;
						e=atof(&Buf[Exp]);	/* Mantisse und Exp. */
						
						if (e<=-20.0 || e>=18.0) return(ExpStr);
					}
#endif
					Raw->d[r++].val.re=atof(Buf);
				}
			}
			continue;
		}
		
		if (toupper(Str[a])>='A' && toupper(Str[a])<='Z')
		{
			short a2;
			/* Variable/Funktion -------------------------------- */
								/* gegf. * einfügen */
			if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
			{
				Raw->t[r]=RT_OP;
				Raw->d[r].op.op='*'; Raw->d[r].op.pri=30;
				r++;
			}
			
			a2=a;		/* Stringzeiger gegf. für Variable sichern */
			
			b=0;
			while (toupper(Str[a])>='A' && toupper(Str[a])<='Z') Buf[b++]=Str[a++];
			if (Str[a]=='²') Buf[b++]=Str[a++];

			Raw->d[r].op.op=0;
								/* Funktion --------------------- */
		
			for (c=b+1;c>1 && !Raw->d[r].op.op; )
			{
				Buf[--c]=0;
				b=0;
			
				while (Function[b])
				{
					if (!stricmp(Function[b],Buf)) Raw->d[r].op.op=b+'A';
					b++;
				}
				if (VEnable) break;
			}
			if (Raw->d[r].op.op)	/* gefunden */
			{
				a=a2+c;
		
				while (Str[a]==' ') a++;	/* wenn Klammer folgt, Pri=60 */
				Raw->d[r].op.pri=(Str[a]=='(')?60:20;
				
				Raw->t[r]=RT_FUNC;
			}
			else				/* Variable ---------------- */
			{
				if (!VList) return(++a2);	/* keine Variablen vorhanden */
				
				a=a2;	/* dann Namen neu einlesen */
			
				b=0;
				while (toupper(Str[a])>='A' && toupper(Str[a])<='Z' || Str[a]>='0' && Str[a]<='9') Buf[b++]=Str[a++];
				Buf[b]=0;
				
				Raw->d[r].var=0;
				
				for (c=b+1; c>1 && !Raw->d[r].var; )
				{
					struct Var *var;

					Buf[--c]=0;
								/* Variable mit diesem Namen vorhanden ? */
					if (var=GetVariable(VList,Buf,0)) Raw->d[r].var=&var->value;
					else if (VEnable) if (!(Raw->d[r].var=&GetVariable(VList,Buf,VAR_USER)->value)) return(-1);
				}
				Raw->t[r]=RT_VAR;
				a=a2+c;
				
				if (!Raw->d[r].var) return(++a2); /* Var. nicht gefunden */
			}
			r++;
			continue;
		}
		
		/* Operator --------------------------------------------- */
		
		if (r==0 || (Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op!=')') || Raw->t[r-1]==RT_FUNC)
		{
			/* mehrere Operatoren aufeinander */
			
			switch (Str[a++])
			{
				case '+' : continue;			/* Plus überlesen */
				case '-' : Raw->t[r]=RT_FUNC;	/* Minus ist Negation */
					Raw->d[r].op.op='n'; Raw->d[r].op.pri=20; break;
				case '\\' : Raw->t[r]=RT_FUNC;	/* Boolsche Negation */
					Raw->d[r].op.op='\\'; Raw->d[r].op.pri=16; break;
				case '?' : Raw->t[r]=RT_FUNC;	/* Boolscher Übersetzer */
					Raw->d[r].op.op='?'; Raw->d[r].op.pri=16; break;
				case '[' :
				case '(' : Raw->t[r]=RT_FUNC; pnest++;
					Raw->d[r].op.op='('; Raw->d[r].op.pri=10; break;
				default: return(a);		/* alles andere Fehler */
			}
		}
		else
		{
			Raw->t[r]=RT_OP;
			switch (Str[a++])	/* normale Rechenoperation */
			{
				case '^' : Raw->d[r].op.op='^'; Raw->d[r].op.pri=40; break;
				case ':' :
				case '/' : Raw->d[r].op.op='/'; Raw->d[r].op.pri=30; break;
				case '*' : Raw->d[r].op.op='*'; Raw->d[r].op.pri=30; break;
				case '+' : Raw->d[r].op.op='+'; Raw->d[r].op.pri=20; break;
				case '-' : Raw->d[r].op.op='-'; Raw->d[r].op.pri=20; break;

				case '\\' :
					if (Str[a]=='=') { a++; Raw->d[r].op.op='u'; Raw->d[r].op.pri=14; }
					else
					{
						/* ggf. * einfügen */
						if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
						{
							Raw->t[r]=RT_OP;
							Raw->d[r].op.op='*'; Raw->d[r++].op.pri=30;
						}
						Raw->t[r]=RT_FUNC;	/* Boolsche Negation */
						Raw->d[r].op.op='\\'; Raw->d[r].op.pri=16;
					}
					break;
				case '?':
					{
						/* ggf. * einfügen */
						if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
						{
							Raw->t[r]=RT_OP;
							Raw->d[r].op.op='*'; Raw->d[r++].op.pri=30;
						}
						Raw->t[r]=RT_FUNC;	/* Boolscher Übersetzer */
						Raw->d[r].op.op='?'; Raw->d[r].op.pri=16;
					}
					break;
				case '<' :
					if (Str[a]=='=') { a++; Raw->d[r].op.op='k'; }
					else { Raw->d[r].op.op='<'; }
					Raw->d[r].op.pri=15;
					break;
				case '>' :
					if (Str[a]=='=') { a++; Raw->d[r].op.op='g'; }
					else { Raw->d[r].op.op='>'; }
					Raw->d[r].op.pri=15;
					break;
				case '=' : Raw->d[r].op.op='='; Raw->d[r].op.pri=14; break;
				case '&' : Raw->d[r].op.op='&'; Raw->d[r].op.pri=13; break;
				case '|' : Raw->d[r].op.op='|'; Raw->d[r].op.pri=12; break;

				case '[' :
				case '(' : pnest++;		/* gegf. * einfügen */
					if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
					{
						Raw->t[r]=RT_OP;
						Raw->d[r].op.op='*'; Raw->d[r++].op.pri=30;
					}
					Raw->d[r].op.op='('; Raw->t[r]=RT_FUNC;
					Raw->d[r].op.pri=10; break;
				case ']' :
				case ')' : pnest--; Raw->d[r].op.op=')'; Raw->d[r].op.pri=10; break;
				case '²' : Raw->t[r]=RT_APP; Raw->d[r].op.op='²'; Raw->d[r].op.pri=50; break;
				case '³' : Raw->t[r]=RT_APP; Raw->d[r].op.op='³'; Raw->d[r].op.pri=50; break;
				case '!' : Raw->t[r]=RT_APP; Raw->d[r].op.op='!'; Raw->d[r].op.pri=50; break;
				case '%' : Raw->t[r]=RT_APP; Raw->d[r].op.op='%'; Raw->d[r].op.pri=50; break;
				case '°' : Raw->t[r]=RT_APP; Raw->d[r].op.op='°'; Raw->d[r].op.pri=50; break;

				default : return(a);
			}
		}
		r++;
		continue;
		
	}
	
	if (pnest) return((short)(a+1));	/* noch Klammern offen ? */
	Raw->t[r]=0;
	Raw->d[r].op.op=Raw->d[r].op.pri=0;
	
							/* Test, ob letztes Zeichen ok. */
	if (Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op!=')' || Raw->t[r-1]==RT_FUNC) return(a);

	return((short)(r?0:-1));
}
