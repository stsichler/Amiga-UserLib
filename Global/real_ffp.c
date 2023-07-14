/* Konversionsroutinen zur Konvertierung von fp in real Zahlen und umgekehrt
 *
 * 5-6-95 ®SSi
 */

/*
 * Aufruf im Hauptprogramm in der Regel:
 *
 *        int error = toreal(real *target, float/double *source);
 *
 *        int error = byreal(float/double *target, real *source);
 *
 * Beide Funktionen sind durch Makros definiert.
 *
 *
 * Format der Real-Zahlen:	(entspr. 68881.ext Format)
 *
 * Sign - 31 Exponent - 64 Mantisse  ( 2,1,1/2,1/4 ...)
 */

#include <user/user.h>

int toreal_ffp(real *r,void *f,int l)
{
	int err=CVERR_OK;
	if (r && f)
	{
		if (l==4)
		{
			ULONG fp=*(ULONG *)f;
			
			if (fp)
			{
			
				r->dat[0]=((fp&0x80)?0xbfffffbe:0x3fffffbe)+(fp&0x7f);
				r->dat[1]=fp&0xffffff00;
				r->dat[2]=0;
			
			} else r->dat[0]=r->dat[1]=r->dat[2]=0;
		} else err=CVERR_PREC;
	} else err=CVERR_NULL;
	return(err);
}

int byreal_ffp(void *f,real *r,int l)
{
	int err=CVERR_OK;
	if (r && f)
	{
		if (l==4)
		{
			ULONG *fp=(ULONG *)f;
			
			if (r->dat[0] || r->dat[1] || r->dat[2])
			{
				if ((r->dat[0]&0x7fffffff)==0)
				{
					err=CVERR_EXP;
					*fp=0;
				}
				else if ((r->dat[0]&0x7fffffff)==0x7fffffff)
				{
					err=CVERR_EXP;
					*fp=(r->dat[0]&(1<<31))?0xffffffff:0xffffff7f;
				}
				else
				{
					ULONG exp=(r->dat[0]&0x7fffffff)-0x3fffffbe;
					*fp=(r->dat[0]&(1<<31))?0x80:0;
					*fp|=r->dat[1]&0xffffff00;
					if (r->dat[1]&0xff || r->dat[2]) err=CVERR_CUT;
					if (exp>0x7f && exp<0x80000000) { err=CVERR_EXP; exp=0x7f; }
					if (exp>=0x80000000) { err=CVERR_EXP; *fp=0; }
					else *fp|=exp;
				}
			} else *fp=0;
		} else err=CVERR_PREC;
	} else err=CVERR_NULL;
	return(err);
}
