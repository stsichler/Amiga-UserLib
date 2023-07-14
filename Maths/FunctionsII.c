/******************************************************************
 * 25-12-93 SSi - Funktionsauswertung // user.lib 21-6-94 ®SSi/94 *
 ******************************************************************/

/* verbesserte Version: iterate() Funktion eingebaut */

#include <proto/exec.h>
#include <exec/memory.h>
#include <error.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <user/functions.h>
#include <user/pointerlists.h>

#define _pi 3.14159265359

#define SPACE "  \r\t\n"

/* Operatorprioritäten ------------------------------------------------ */

/*		Fkt()       60
 *    ! ²	³ %	°   50		(%: 50% = .5 ) ( 180° = ¶)
 *		^           40
 *		*  :  /     30
 *		+  -  n	Fkt 20		(n als Negation)
 *		\  ?        16    (\ als Boolsche Negation) (? als Boolscher Übersetzer)
 *		< <= > >=   15    (<= = k / >= = g)
 *    = \=        14    (= als Boolscher Vergleich) (\= = u)
 *    &           13    (Boolsches Und)
 *    |           12    (Boolsches Oder)
 *		()          10
 *		=            0
 *
 */

/* private Prototypen ------------------------------------------------------ */

struct Var *GetVariableFromPool(fnctcode *,char *);

short _new_strtofnctraw(char *,fnctraw *,fnctcode *);

/* ------------------------------------------------------------------------- */

short makefnctcode(char _str[],fnctcode *code)
/* wandelt den String ins Raw-Format um (Leerzeichen, \t, \n, u. \r werden
 * überlesen. 
 * RETURN: 0 bei o.k. / -1 bei allgem. Fehler / Zeichen+1 bei fehlerhaftem
 *         Zeichen (Syntax Error o.ä.)
 *
 * ACHTUNG: das fnctcode-Feld muß von allocfnctcode() kommen und unbenutzt
 *          sein!
 */
 
{
	char *str;
	short i,ia,pc,err=0,str_len;
	char *p;
	PLIST ss=0;	/* SubStrings */
	PLIST vn=0;	/* Variablennamen */
	PLIST nr=0;	/* normales Rawfeld */
	PLIST ir=0;	/* init Rawfeld */
	PLIST vr=0;	/* Varraw */ 
	
	if (!(code && _str[0])) return((short)-1);
	
	if (!(str=strdup(_str))) return((short)-1);

	str_len=strlen(str);
	
	/* str in Einzelstrings zerteilen - - - - - - - - - - - - - - */
	
	for (i=ia=0,pc=0;str[i];i++)
	{
		if (strchr("([",str[i])) pc++;
		if (strchr(")]",str[i])) pc--;
		
		if (!pc)
		{
			if (str[i]==';')
			{
				str[i]=0;
				PLAddHead(&ss,&str[ia]);
				ia=i+1;
			}
		}
	}
	PLAddHead(&ss,&str[ia]);
	
	/* Einzelstrings auswerten - - - - - - - - - - - - - - - - - */
	
	for (i=0;p=(char *)PLGetPos(&ss,i);i++)
	{
		short xa=0,xb=0;
		
		if (i!=PLGetNum(&ss)-1)	/* letzter Zeiger (=Zielterm) ? */
		{		/* nein ! */
			while (p[xa] && strchr(SPACE,p[xa])) xa++;	/* Variablennamen herausfinden */
			
			xb=xa;
			while (toupper(p[xb])>='A' && toupper(p[xb])<='Z' || p[xb]=='_') xb++;
			if (p[xb]=='=')
			{
				p[xb++]=0;
			}
			else
			{
				p[xb++]=0;
				while(p[xb] && strchr(SPACE,p[xb])) xb++;
				if (p[xb++]!='=') { err=xb;goto __END; }
			}
				
			/* -> p[xa] zeigt auf den Var.Namen / p[xb] auf norm_term */
			
			PLAddTail(&vn,&p[xa]);
			PLAddTail(&nr,&p[xb]);
	
			if (code->flags&FCF_ITERATIVE)
			{
				/* init. Term suchen */
	
				p=&p[xb];	/* ->p zeigt auf norm_term */
				xa=strlen(p)-1;
				if (xa<0) { err=(int)p-(int)str+1; goto __END; }
				
				while(p[xa] && strchr(SPACE,p[xa])) xa--;
				if (p[xa]==']')
				{
					p[xa]=0;
					for (pc=-1;xa>=0 && pc;xa--)
					{
						if (p[xa] && strchr("([",p[xa])) pc++;
						if (p[xa] && strchr(")]",p[xa])) pc--;
					}
					/* ->p[xa+2] zeigt auf init_term */
					
					if (xa>=0) { PLAddTail(&ir,&p[xa+2]); p[xa+1]=0; }
					else { err=(int)p-(int)str+strlen(p)+1; goto __END; }
				}
				else PLAddTail(&ir,0);
			}
		}
		else PLAddTail(&nr,p);
	}
	
	/* Einzelterme auswerten (aus den PLISTs) - - - - - - - - - - - - - */
	
	for (ia=0;ia<3;ia++)	/* Arbeitsschrittschleife: */
	/* ia=0: Anlegen der fnctraw_entrys
	 *    1: Aufstellen der Init(bei Iter.) bzw. Norm(ohne Iter.)-Rawfelder 
	 *        + Anlegen der Variablen
	 *    2: Aufstellen der Norm(bei Iter.)-Rawfelder + Aufstellen des Zielterms
	 */
	{
		for (i=0;i<PLGetNum(&nr);i++)
		{		
			switch (ia)
			{
				case 0:
				if (i!=PLGetNum(&nr)-1)
				{
					struct fnctraw_entry *fre;
					if (!(fre=calloc(1,sizeof(struct fnctraw_entry)))) { err=-1; goto __END; }
					PLAddTail(&vr,fre);
					AddTail((struct List *)&code->varraws,(struct Node *)&fre->node);
				} break;
				case 1:
				if (i!=PLGetNum(&nr)-1)
				{
					struct fnctraw_entry *fre=(struct fnctraw_entry *)PLGetPos(&vr,i);
					short e;
					char *p;
					struct Var *var;
					
					if (p=(char *)PLGetPos((code->flags&FCF_ITERATIVE)?&ir:&nr,i))
					{
						fnctraw **frz;
						frz=(code->flags&FCF_ITERATIVE)?&fre->fraw_init:&fre->fraw;
						if (!(*frz=allocfnctraw(strlen(p)*2+4))) { err=-1; goto __END; }
						if (e=_new_strtofnctraw(p,*frz,code)) { err=(e==-1)?-1:(e+(int)p-(int)str); goto __END; }
					}
								
					p=(char *)PLGetPos(&vn,i);
					if (var=GetVariable(&code->local_varpool,p,0)) { err=(int)p-(int)str+1;goto __END; }
					if (!(var=GetVariable(&code->local_varpool,p,VAR_USER))) { err=-1; goto __END; }
					
					fre->var=var;
				} break;
				case 2:
				if (i!=PLGetNum(&nr)-1)
				{
					struct fnctraw_entry *fre=(struct fnctraw_entry *)PLGetPos(&vr,i);
					short e;
					char *p;
					
					if ((p=(char *)PLGetPos(&nr,i)) && (code->flags&FCF_ITERATIVE))
					{
						if (!(fre->fraw=allocfnctraw(strlen(p)*2+4))) { err=-1; goto __END; }
						if (e=_new_strtofnctraw(p,fre->fraw,code)) { err=(e==-1)?-1:(e+(int)p-(int)str); goto __END; }
					}
				}
				else
				{
					short e;
					char *p;
					
					if (p=(char *)PLGetPos(&nr,i))
					{
						if (!(code->targetraw=allocfnctraw(strlen(p)*2+4))) { err=-1; goto __END; }
						if (e=_new_strtofnctraw(p,code->targetraw,code)) { err=(e==-1)?-1:(e+(int)p-(int)str); goto __END; }
					}
				}	break;			
			}
		}
	}
	if (!code->targetraw) { err=-1; goto __END; }
	if (code->flags&FCF_ITERATIVE)
	{
		if (!(code->brkvar=GetVariable(&code->local_varpool,"brk",0))) err=str_len+1;
	}
__END:
	PLClear(&ss);	
	PLClear(&vn);
	PLClear(&nr);
	PLClear(&ir);
	PLClear(&vr);
	free(str);
	return(err);
}

/* ------------------------------------------------------------------------- */

short _new_strtofnctraw(Str,Raw,Code)	
/* StringToFunctionRaw */
char Str[];            /* wandelt den String ins Raw-Format um */
fnctraw *Raw;          /* Zeiger auf das zu produzierende Raw-Feld */
fnctcode *Code;				 /* ^fnctcode (wegen Variablen benötigt) */
/* RETURN: 0 - ok. / !=0 Nummer des fehlerhaften */
/* Zeichens +1 !!!! */
{
	short a,b,c,r;	/* oder -1 bei allgem. Fehler */
	short pnest=0;
	char Buf[64];
 
	/* Funktionen ------ */

	static char *Function[] =
	{
		"sin","cos","tan","asin","acos","atan","abs",	// A-G
		"cot","sinh","cosh","tanh","sqrt","ln","log",	// H-N
		"exp","sin²","cos²","tan²","sgn","iterate",	// O-T
		"arg","re","im","conj",0								// U-X
	};
	
	a=r=0;
	
	while (Str[a])
	{
		if (strchr(SPACE,Str[a])) { a++; continue; }
		
		if (pnest<0) return(a);	/* mehr Klammern zu als offen ? */
		
		if (strchr("01234567890.¶¼½¾",Str[a]))
		{
			/* Explizite Zahl ----------------------------------- */
								/* gegf. * einfügen */
			if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_CODE || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
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
		
		if (toupper(Str[a])>='A' && toupper(Str[a])<='Z' || Str[a]=='_')
		{
			short a2;
			/* Variable/Funktion -------------------------------- */
								/* gegf. * einfügen */
			if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_CODE || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
			{
				Raw->t[r]=RT_OP;
				Raw->d[r].op.op='*'; Raw->d[r].op.pri=30;
				r++;
			}
			
			a2=a;		/* Stringzeiger gegf. für Variable sichern */
			
			b=0;
			while (toupper(Str[a])>='A' && toupper(Str[a])<='Z' || Str[a]=='_') Buf[b++]=Str[a++];
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
			}
			if (Raw->d[r].op.op)	/* gefunden */
			{
				a=a2+c;
		
				while (Str[a] && strchr(SPACE,Str[a])) a++;/* wenn Klammer folgt, Pri=60 */
				Raw->d[r].op.pri=(Str[a]=='(')?60:20;
				
				Raw->t[r]=RT_FUNC;
				
				if (Raw->d[r].op.op=='T') /* iterate() - - - - - - - - - */
				{
					short i=0;
					short aa=a,err;
					char *Str2;
					fnctcode *cc;
					
					if (Raw->d[r].op.pri!=60) return(a++); /* keine Klammer ?! */
					do						/* doch, inneren Klammerbereich überspringen */
					{
						if (Str[a]=='(') i++;
						if (Str[a]==')') i--;
					} while (Str[++a] && i);
					if (i) return(a++);
					Str[a-1]=0;
					
					if (!(Str2=strdup(&Str[++aa]))) return((short)-1);
					
					/* Str2 ist nun die innere Funktion der iterate() Fnkt. */
										
					if (!(cc=allocfnctcode(0,Code->brkaddr))) return((short)-1);
					cc->parent=Code;
					cc->flags=FCF_ITERATIVE;
					PLAddHead(&Code->used_codes,cc);
					Raw->t[r]=RT_CODE;
					Raw->d[r].code=cc;
					
					if (err=makefnctcode(Str2,cc)) return((short)((err==-1)?err:(err+aa)));
				}
			}
			else				/* Variable ---------------- */
			{
				a=a2;	/* dann Namen neu einlesen */
			
				b=0;
				while (toupper(Str[a])>='A' && toupper(Str[a])<='Z' || Str[a]>='0' && Str[a]<='9' || Str[a]=='_') Buf[b++]=Str[a++];
				Buf[b]=0;
				
				Raw->d[r].var=0;
				
				for (c=b+1; c>1 && !Raw->d[r].var; )
				{
					struct Var *var;

					Buf[--c]=0;
								/* Variable mit diesem Namen vorhanden ? */
					if (var=GetVariableFromPool(Code,Buf)) Raw->d[r].var=&var->value;
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
						if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_CODE || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
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

				case '(' : pnest++;		/* gegf. * einfügen */
					if (r>0 && (Raw->t[r-1]==RT_VAL || Raw->t[r-1]==RT_VAR || Raw->t[r-1]==RT_APP || Raw->t[r-1]==RT_CODE || Raw->t[r-1]==RT_OP && Raw->d[r-1].op.op==')'))
					{
						Raw->t[r]=RT_OP;
						Raw->d[r].op.op='*'; Raw->d[r++].op.pri=30;
					}
					Raw->d[r].op.op='('; Raw->t[r]=RT_FUNC;
					Raw->d[r].op.pri=10; break;
					
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

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

struct Var *GetVariableFromPool(fnctcode *code,char *name)
/* sucht die Var. in den Pools des Codefelds */
/* RETURN: Addr. bei gefunden, sonst null */
{
	struct Var *v=0;
	
	if (!code) return(0);

	v=GetVariable(&code->local_varpool,name,0);
	if (!v) v=GetVariable(code->global_varpool,name,0);
	
	if (!v) v=GetVariableFromPool(code->parent,name);

	return(v);
}
