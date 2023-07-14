/* 25-12-94 SSi - Funktionsauswertung // user.lib 21-6-94 ®SSi\94 */

#include <error.h>
#include <math.h>
#include <stdlib.h>
#include <proto/exec.h>

#include <user/functions.h>

#define _pi 3.14159265359

int pow_complex(complex *result, complex *base, complex *exp);
int ln_complex(complex *result, complex *val);
int __inline exp_complex(complex *result, complex *val);
double __inline abs_complex(complex *val);
double __inline arg_complex(complex *val);

/* Berechnungsroutine für fnctcode Felder ------------------------------ */


int evalfnctcode(complex *result,fnctcode *code)
/* berechnet (evtl. auch iterativ) das FnctCode - Feld.
 * RETURN: Wert des Terms.
 *         Bei Berechnungsfehler jeglicher Art ist _FPERR ungleich null.
 */
{
	unsigned long brkaddr_inh=*code->brkaddr;
	struct fnctraw_entry *fre;
	short first_iter=-1;
	
	do	/* Iterationsschleife */
	{
		/* Variablenterme errechnen */
		
		if (first_iter && code->brkvar)	/* Init. Terme berechnen */
		{
			for(fre=(struct fnctraw_entry *)code->varraws.mlh_Head;
			    fre->node.mln_Succ;
					fre=(struct fnctraw_entry *)fre->node.mln_Succ)
			{
				if (fre->fraw_init) evalfnctraw(&fre->var->value,fre->fraw_init);
				else SetVariable(fre->var,(double)0.0,(double)0.0);
				if (_FPERR) return(_FPERR);
			}
		}
		else	/* normale Terme errechnen */
		{
			for(fre=(struct fnctraw_entry *)code->varraws.mlh_Head;
			    fre->node.mln_Succ;
					fre=(struct fnctraw_entry *)fre->node.mln_Succ)
			{
				if (evalfnctraw(&fre->var->value,fre->fraw)) return(_FPERR);
			}
		}		
		
		if (*code->brkaddr!=brkaddr_inh) { return(_FPERR=_FPEbrk); }
		first_iter=0;	
	} 
	while (code->brkvar && !(code->brkvar->value.re || code->brkvar->value.im));

	return(evalfnctraw(result,code->targetraw));
}

/* --------------------------------------------------------------------- */
/* BERECHNUNGSROUTINE (für fnctraw Felder) ----------------------------- */
/* --------------------------------------------------------------------- */

#define Clear() { RDS[0].op.op=0;RDS[0].op.pri=0;RDSP=1; }
#define Put(raw) RDS[RDSP++]=raw
#define Raw(offset) RDS[RDSP+(offset)]

int evalfnctraw(result, Raw)	/* berechnet den Wert des Raw-Felds */
complex *result;				/* ACHTUNG: externe Variable _FPERR wird */
fnctraw *Raw;		/* !=0, wenn der Ausdruck unberechenbar */
{						/* war. , RETURN: _FPERR */
	short a;
	rawdata *RDS;					/* RawData - Stack */
	short RDSP;						/* RawData - Stack-Pointer */
	rawtype *RTF;
	rawdata *RDF;
	
	if (!Raw) return(_FPERR=_FPEnfr);
		
	RTF=Raw->t;
	RDF=Raw->d;
	
	RDS=Raw->ds;
	
	Clear();
	_FPERR=0;
	
	for(a=0;;)
	{
		if (_FPERR) return(_FPERR);
		
		switch (RTF[a])	/* Zahl/Variable --------------------------- */
		{
			case RT_VAL: 
				Put(RDF[a++]); break;	/* übertragen */
			case RT_VAR: 
				Raw(0).val.re=RDF[a].var->re;
				Raw(0).val.im=RDF[a++].var->im;
				RDSP++; break;
	
			/* Operator -------------------------------------------- */
			
			case RT_FUNC: Put(RDF[a++]); break;
			case 0: 
			case RT_OP:
			case RT_APP:
				if (RDF[a].op.pri>Raw(-2).op.pri)
				{
					if (RTF[a]==RT_OP)	/* Rechenop. */
					{
						Put(RDF[a]);
						if ((RDF[a].op.op=='*' || RDF[a].op.op=='&') && !Raw(-2).val.re && !Raw(-2).val.im || RDF[a].op.op=='|' && (Raw(-2).val.re || Raw(-2).val.im)) 
						                                /* bei 0*... zweiten Operanden */
						{                               /* überspringen */
							short pnest;
							for(pnest=0,a++;RTF[a] && (pnest || !(RTF[a]==RT_OP && RDF[a].op.pri<=Raw(-1).op.pri));a++)
							{
								if (RTF[a]==RT_FUNC && RDF[a].op.op=='(') pnest++;
								if (RTF[a]==RT_OP && RDF[a].op.op==')') pnest--;
							}
							if (Raw(-1).op.op=='|') { Raw(-2).val.re=1.0; Raw(-2).val.im=0.0; }
							RDSP--;	/* '*' vom Stack löschen */
							continue;
						}
						a++;
					}
					else
					{
						complex val=Raw(-1).val;
						switch(RDF[a++].op.op)	/* Appendix-operator */
						{
							case '²' : { Raw(-1).val.re=val.re*val.re-val.im*val.im; Raw(-1).val.im=2.0*val.re*val.im; } break;
							case '³' : if (!val.im) { Raw(-1).val.re=val.re*val.re*val.re; }
							           else { Raw(-1).val.re=val.re*val.re*val.re-3.0*val.re*val.im*val.im; Raw(-1).val.im=3.0*val.re*val.re*val.im-val.im*val.im*val.im; }
							           break;
							case '%' : Raw(-1).val.re/=100.0; Raw(-1).val.im/=100.0; break;
							case '°' : Raw(-1).val.re*=(_pi/180.0); Raw(-1).val.im*=(_pi/180.0); break;
							case '!' :
							{
								int fac;	/* Fakultät */
								
								if (Raw(-1).val.im) return(_FPERR=_FPEnfv);
								if (Raw(-1).val.re=(double)(fac=(int)(Raw(-1).val.re+.5)))
								{
									if (fac<0) return(_FPERR=_FPEnfv);
									while (--fac) Raw(-1).val.re*=(double)fac;
								}
								else Raw(-1).val.re=(double)1;
							}
							break;
						}
					}
				}
				else
				{		/* der Ausdruck auf dem Stack muß errechnet werden */
					complex val=Raw(-3).val;
					
					switch (Raw(-2).op.op)
					{
						case '+': RDSP-=2; Raw(-1).val.re=val.re+Raw(1).val.re; Raw(-1).val.im=val.im+Raw(1).val.im; break;
						case '-': RDSP-=2; Raw(-1).val.re=val.re-Raw(1).val.re; Raw(-1).val.im=val.im-Raw(1).val.im; break;
						case 'n': RDSP-=1; Raw(-1).val.re=-Raw(0).val.re; Raw(-1).val.im=-Raw(0).val.im; break;
						case '*': RDSP-=2; Raw(-1).val.re=val.re*Raw(1).val.re-val.im*Raw(1).val.im; Raw(-1).val.im=val.re*Raw(1).val.im+val.im*Raw(1).val.re; break;
						case '/': RDSP-=2;
							{
								double div=Raw(1).val.re*Raw(1).val.re+Raw(1).val.im*Raw(1).val.im;
								if (div==0.0) _FPERR=_FPEZDV;
								else
								{
									Raw(-1).val.re=(val.re*Raw(1).val.re+val.im*Raw(1).val.im)/div;
									Raw(-1).val.im=(val.im*Raw(1).val.re-val.re*Raw(1).val.im)/div;
								}
							}
							break;
						case '^': RDSP-=2;
							if (pow_complex(&Raw(-1).val,&Raw(-1).val,&Raw(1).val)) return(_FPERR);
							break;

						case '(':
							RDSP-=1;
							Raw(-1).val=Raw(0).val;
							if (RDF[a].op.op==')') a++;
							else _FPERR=_FPEubp;
							break;
						
						case 0: result->re=Raw(-1).val.re; result->im=Raw(-1).val.im; return(0);

						case 'A': RDSP-=1; 		// sin
							if (Raw(0).val.im)
							{
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								Raw(-1).val.re=(e1+e2)*0.5*sin(Raw(0).val.re);
								Raw(-1).val.im=(e1-e2)*0.5*cos(Raw(0).val.re);
							}
							else
							{
								Raw(-1).val.re=sin(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'B': RDSP-=1; 		// cos
							if (Raw(0).val.im)
							{
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								Raw(-1).val.re=(e2+e1)*0.5*cos(Raw(0).val.re);
								Raw(-1).val.im=(e2-e1)*0.5*sin(Raw(0).val.re);
							}
							else
							{
								Raw(-1).val.re=cos(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'C': RDSP-=1;		// tan
							if (Raw(0).val.im)
							{
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								double div=4.0*e1*e2*cos(Raw(0).val.re)*cos(Raw(0).val.re)+(e1-e2)*(e1-e2);
								
								if (div)
								{
									Raw(-1).val.re=4.0*e1*e2*sin(Raw(0).val.re)*cos(Raw(0).val.re)/div;
									Raw(-1).val.im=(e1+e2)*(e1-e2)/div;
								}
								else _FPERR=_FPEudf;
							}
							else
							{
								Raw(-1).val.re=tan(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'D': RDSP-=1;		// asin
							if (Raw(0).val.im || Raw(0).val.re<-1.0 || Raw(0).val.re>1.0)
							{
								double nrt,prt,np;
								np=Raw(0).val.re*Raw(0).val.re+Raw(0).val.im*Raw(0).val.im+1.0;
								nrt=sqrt(np-2.0*Raw(0).val.re);
								prt=sqrt(np+2.0*Raw(0).val.re);
								Raw(-1).val.re=asin((prt-nrt)*0.5);
								Raw(-1).val.im=log((sqrt(2.0*(nrt*prt+np-2.0))+prt+nrt)*0.5)*((Raw(0).val.im<=0.0)?-1.0:1.0);
							}
							else
							{
								Raw(-1).val.re=asin(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'E': RDSP-=1;	// acos
							if (Raw(0).val.im || Raw(0).val.re<-1.0 || Raw(0).val.re>1.0)
							{
								double nrt,prt,np;
								np=Raw(0).val.re*Raw(0).val.re+Raw(0).val.im*Raw(0).val.im+1.0;
								nrt=sqrt(np-2.0*Raw(0).val.re);
								prt=sqrt(np+2.0*Raw(0).val.re);
								Raw(-1).val.re=acos((prt-nrt)*0.5);
								Raw(-1).val.im=log((sqrt(2.0*(nrt*prt+np-2.0))+prt+nrt)*0.5)*((Raw(0).val.im<=0.0)?1.0:-1.0);
							}
							else
							{
								Raw(-1).val.re=acos(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'F': RDSP-=1; 		// atan
							if (Raw(0).val.im)
							{
								double s=Raw(0).val.re*Raw(0).val.re+Raw(0).val.im*Raw(0).val.im;
								double div=s+1.0-2.0*Raw(0).val.im;
								double l,t;
								if (div)
								{
									t=2.0*Raw(0).val.re;
									if (t) Raw(-1).val.re=0.5*atan((s-1.0)/t)+((Raw(0).val.re<0.0)?(-_pi/4.0):(_pi/4.0));
									else Raw(-1).val.re=(Raw(0).val.re<0.0)?0.0:(0.5*_pi);
									l=(s+1.0+2.0*Raw(0).val.im)/(s+1.0-2.0*Raw(0).val.im);
									if (l) Raw(-1).val.im=0.25*log(l);
									else _FPERR=_FPEudf;
								}
								else _FPERR=_FPEudf;
							}
							else
							{
								Raw(-1).val.re=atan(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'G': RDSP-=1; Raw(-1).val.re=abs_complex(&Raw(0).val); break;
						case 'H': RDSP-=1; 		// cot
							if (Raw(0).val.im)
							{
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								double div=4.0*e1*e2*cos(Raw(0).val.re)*cos(Raw(0).val.re)-(e1+e2)*(e1+e2);
								
								if (div)
								{
									Raw(-1).val.re=-4.0*e1*e2*sin(Raw(0).val.re)*cos(Raw(0).val.re)/div;
									Raw(-1).val.im=(e1+e2)*(e1-e2)/div;
								}
								else _FPERR=_FPEudf;
							}
							else
							{
								Raw(-1).val.re=cot(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'I': RDSP-=1; 		// sinh
							if (Raw(0).val.im)
							{
								complex buf;
								if (!exp_complex(&Raw(-1).val,&Raw(0).val))
								{
									Raw(0).val.re*=-1.0; Raw(0).val.im*=-1.0;
									if (!exp_complex(&buf,&Raw(0).val))
									{
										Raw(-1).val.re=(Raw(-1).val.re-buf.re)*0.5;
										Raw(-1).val.im=(Raw(-1).val.im-buf.im)*0.5;
									}
								}
							}
							else
							{

#ifdef _FFP
								if (Raw(0).val.re<-44.361) { Raw(-1).val.re=-9.223E+18; }
								else if (Raw(0).val.re>44.361) { Raw(-1).val.re=9.223E+18; }
								else 
#endif						
								Raw(-1).val.re=sinh(Raw(0).val.re);
								Raw(-1).val.im=0.0; 
							}
							break;
						case 'J': RDSP-=1; 		// cosh
							if (Raw(0).val.im)
							{
								complex buf;
								if (!exp_complex(&Raw(-1).val,&Raw(0).val))
								{
									Raw(0).val.re*=-1.0; Raw(0).val.im*=-1.0;
									if (!exp_complex(&buf,&Raw(0).val))
									{
										Raw(-1).val.re=(Raw(-1).val.re+buf.re)*0.5;
										Raw(-1).val.im=(Raw(-1).val.im+buf.im)*0.5;
									}
								}								
							}
							else
							{
#ifdef _FFP							
								if (abs(Raw(0).val.re)>44.361) { Raw(-1).val.re=9.223E+18; }
								else 
#endif
								Raw(-1).val.re=cosh(Raw(0).val.re);
								Raw(-1).val.im=0.0;
							}
							break;
						case 'K': RDSP-=1; 		// tanh
							if (Raw(0).val.im)
							{
								complex e1,e2;
								double div;
								exp_complex(&e1,&Raw(0).val);
								Raw(0).val.re*=-1.0; Raw(0).val.im*=-1.0;
								exp_complex(&e2,&Raw(0).val);
								div=(e1.re+e2.re)*(e1.re+e2.re)+(e1.im+e2.im)*(e1.im+e2.im);
								if (div)
								{
									Raw(-1).val.re=((e1.re-e2.re)*(e1.re+e2.re)+(e1.im-e2.im)*(e1.im+e2.im))/div;
									Raw(-1).val.im=((e1.im-e2.im)*(e1.re+e2.re)-(e1.re-e2.re)*(e1.im+e2.im))/div;
								} else _FPERR=_FPEovf;
							}
							else Raw(-1).val.re=tanh(Raw(0).val.re); 
							break;
						case 'L': RDSP-=1;		// ^
							{
								static complex a_half = { 0.5,0.0 };
								pow_complex(&Raw(-1).val,&Raw(0).val,&a_half);
							}
							break;
						case 'M': RDSP-=1; ln_complex(&Raw(-1).val,&Raw(0).val); break;
						case 'N': RDSP-=1; 		// log10
							ln_complex(&Raw(-1).val,&Raw(0).val);
							if (!_FPERR)
							{
								Raw(-1).val.re/=/*ln(10.0)*/2.30258509299;
								Raw(-1).val.im/=/*ln(10.0)*/2.30258509299;
							}
							break;
						case 'O': RDSP-=1; exp_complex(&Raw(-1).val,&Raw(0).val); break;
						case 'P': RDSP-=1; 		// sin²
							if (Raw(0).val.im)
							{
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								complex buf;
								buf.re=(e1+e2)*0.5*sin(Raw(0).val.re);
								buf.im=(e1-e2)*0.5*cos(Raw(0).val.re);
								Raw(-1).val.re=buf.re*buf.re-buf.im*buf.im;
								Raw(-1).val.im=2.0*buf.re*buf.im;
							}
							else
							{ double z; z=sin(Raw(0).val.re); Raw(-1).val.re=z*z; Raw(-1).val.im=0.0; } 
							break;
						case 'Q': RDSP-=1;		// cos²
							if (Raw(0).val.im)
							{
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								complex buf;
								buf.re=(e2+e1)*0.5*cos(Raw(0).val.re);
								buf.im=(e2-e1)*0.5*sin(Raw(0).val.re);
								Raw(-1).val.re=buf.re*buf.re-buf.im*buf.im;
								Raw(-1).val.im=2.0*buf.re*buf.im;
							}
							else
							{ double z; z=cos(Raw(0).val.re); Raw(-1).val.re=z*z; Raw(-1).val.im=0.0; } 
							break;
						case 'R': RDSP-=1;		// tan²
							if (Raw(0).val.im)
							{
								complex buf;
								double e1=exp(Raw(0).val.im), e2=exp(-Raw(0).val.im);
								double div=4.0*e1*e2*cos(Raw(0).val.re)*cos(Raw(0).val.re)+(e1-e2)*(e1-e2);
								
								if (div)
								{
									buf.re=4.0*e1*e2*sin(Raw(0).val.re)*cos(Raw(0).val.re)/div;
									buf.im=(e1+e2)*(e1-e2)/div;
									Raw(-1).val.re=buf.re*buf.re-buf.im*buf.im;
									Raw(-1).val.im=2.0*buf.re*buf.im;
								}
								else _FPERR=_FPEudf;
							}
							else
							{ double z; z=tan(Raw(0).val.re); Raw(-1).val.re=z*z; Raw(-1).val.im=0.0; } 
							break;
						case 'S': RDSP-=1; 
							if (Raw(0).val.im) _FPERR=_FPEudf;
							else
							{
								if (Raw(0).val.re==0.0) Raw(-1).val.re=0.0;
								else Raw(-1).val.re=(Raw(0).val.re<0)?-1.0:1.0;
								Raw(-1).val.im=0.0;
							}
							break;
						case 'U': RDSP-=1; Raw(-1).val.re=arg_complex(&Raw(0).val); Raw(-1).val.im=0.0; break;
						case 'V': RDSP-=1; Raw(-1).val.re=Raw(0).val.re; Raw(-1).val.im=0.0; break;
						case 'W': RDSP-=1; Raw(-1).val.re=Raw(0).val.im; Raw(-1).val.im=0.0; break;
						case 'X': RDSP-=1; Raw(-1).val.re=Raw(0).val.re; Raw(-1).val.im=-Raw(0).val.im; break;
						case '<': RDSP-=2; 
							if (Raw(-1).val.im||Raw(1).val.im) _FPERR=_FPEudf;
							else Raw(-1).val.re=(Raw(-1).val.re<Raw(1).val.re)?1.0:0.0;
							break;
						case 'k': RDSP-=2; 
							if (Raw(-1).val.im||Raw(1).val.im) _FPERR=_FPEudf;
							else Raw(-1).val.re=(Raw(-1).val.re<=Raw(1).val.re)?1.0:0.0; 
							break;
						case '>': RDSP-=2;
							if (Raw(-1).val.im||Raw(1).val.im) _FPERR=_FPEudf;
							else Raw(-1).val.re=(Raw(-1).val.re>Raw(1).val.re)?1.0:0.0;
							break;
						case 'g':
							if (Raw(-1).val.im||Raw(1).val.im) _FPERR=_FPEudf;
							else RDSP-=2; Raw(-1).val.re=(Raw(-1).val.re>=Raw(1).val.re)?1.0:0.0;
							break;
						case '=': RDSP-=2; Raw(-1).val.re=(Raw(-1).val.re==Raw(1).val.re && Raw(-1).val.im==Raw(1).val.im)?1.0:0.0; Raw(-1).val.im=0.0; break;
						case 'u': RDSP-=2; Raw(-1).val.re=(Raw(-1).val.re==Raw(1).val.re && Raw(-1).val.im==Raw(1).val.im)?0.0:1.0; Raw(-1).val.im=0.0; break;
						case '\\': RDSP-=1; Raw(-1).val.re=(Raw(0).val.re || Raw(0).val.im)?0.0:1.0; Raw(-1).val.im=0.0; break;
						case '&': RDSP-=2; Raw(-1).val.re=((Raw(-1).val.re || Raw(-1).val.im) && (Raw(1).val.re || Raw(1).val.im))?1.0:0.0; Raw(-1).val.im=0.0; break;
						case '|': RDSP-=2; Raw(-1).val.re=(Raw(-1).val.re || Raw(-1).val.im || Raw(1).val.re || Raw(1).val.im)?1.0:0.0; Raw(-1).val.im=0.0; break;
						case '?': RDSP-=1; Raw(-1).val.re=(Raw(0).val.re || Raw(0).val.im)?1.0:0.0; Raw(-1).val.im=0.0; break;

						default: return(_FPERR=_FPEukn);
					}
				}
				break;
						
			/* Code ------------------------------------------------ */
	
			case RT_CODE:
				evalfnctcode(&Raw(0).val,RDF[a++].code);
				RDSP++; break;

			/* ----------------------------------------------------- */
			
			default: return(_FPERR=_FPEukn);
		}
	}
}

/* Routinen zur Rechnung mit komplexen Zahlen ----------------------------------------- */

int pow_complex(complex *result, complex *base, complex *ex)
{
	complex exp1,exp2;
	if (!base->re && !base->im)
	{
		result->re=result->im=0.0;
		return(_FPERR);
	}
	if (!base->im && !ex->im && base->re>0.0)
	{
		result->re=pow(base->re,ex->re);
		result->im=0.0;
		return(_FPERR);
	}
	if (!ln_complex(&exp1,base))
	{
		exp2.re=exp1.re*ex->re-exp1.im*ex->im;
		exp2.im=exp1.re*ex->im+exp1.im*ex->re;
		return(exp_complex(result,&exp2));
	}
	return(_FPERR);
}

int ln_complex(complex *result, complex *val)
{
	double absval;
	if (val->im || val->re<0.0)
	{
		absval=abs_complex(val);
		result->re=log(absval);
		result->im=acos(val->re/absval);
		if (val->im<0.0) result->im*=-1.0;
		return(_FPERR);
	}
	else
	{
		if (val->re)
		{
			result->re=log(val->re);
			result->im=0.0;
			return(_FPERR);
		}
		else return(_FPERR=_FPEzlv);
	}
}

int __inline exp_complex(complex *result, complex *val)
{
	result->re=exp(val->re);
	if (val->im)
	{
		result->im=result->re*sin(val->im);
		result->re*=cos(val->im);
	}
	else result->im=0.0;
	return(0);
}

double __inline abs_complex(complex *val)
{
	if (!val->im) return(fabs(val->re));
	if (!val->re) return(fabs(val->im));
	return(sqrt(val->re*val->re+val->im*val->im));
}

double __inline arg_complex(complex *val)
{
	double result;
	result=acos(val->re/abs_complex(val));
	if (val->im<0.0) result*=-1.0;
	return(result);
}
