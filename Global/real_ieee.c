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
 * Sign - 31 Exponent - 64 Mantisse ( 2,1,1/2,1/4 ...)
 */

#include <user/user.h>


int toreal_ieee(real *r,void *f,int l)
{
	int err=CVERR_OK;
	if (r && f)
	{
		switch (l)
		{
			case 4:	// float
			{
				ULONG fp=*(ULONG *)f;
				
				if (fp)
				{
					ULONG exp=(fp&0x7f800000)>>23;
					
					if (!exp) r->dat[0]=0;
					else if (exp==0xff) r->dat[0]=0x7fffffff;
					else r->dat[0]=0x3fffff80+exp;
					r->dat[0]|=fp&(1<<31);
					r->dat[1]=(1<<31)|((fp&0x007fffff)<<8);
					r->dat[2]=0;
					
				} else r->dat[0]=r->dat[1]=r->dat[2]=0;
			} break;
			case 8: // double
			{
				ULONG fph=((ULONG *)f)[0],fpl=((ULONG *)f)[1];
				
				if (fph || fpl)
				{
					ULONG exp=(fph&0x7ff00000)>>20;
					
					if (!exp) r->dat[0]=0;
					else if (exp==0x7ff) r->dat[0]=0x7fffffff;
					else r->dat[0]=0x3ffffc00+exp;
					r->dat[0]|=fph&(1<<31);
					r->dat[1]=(1<<31)|((fph&0x000fffff)<<11)|((fpl&0xffe00000)>>21);
					r->dat[2]=(fpl&0x001fffff)<<11;
					
				} else r->dat[0]=r->dat[1]=r->dat[2]=0;
			} break;
			case 12: // extend (nicht implementiert)
			{
				ULONG *fp=(ULONG *)f;
				r->dat[0]=fp[0];
				r->dat[1]=fp[1];
				r->dat[2]=fp[2];
			} break;
			default: err=CVERR_PREC;
		}
	} else err=CVERR_NULL;
	return(err);
}

int byreal_ieee(void *f,real *r,int l)
{
	int err=CVERR_OK;
	if (r && f)
	{
		switch (l)
		{
			case 4:	// float
			{
				ULONG *fp=(ULONG *)f;
				
				if (r->dat[0] || r->dat[1] || r->dat[2])
				{
					ULONG exp=r->dat[0]&0x7fffffff;
					if (exp==0) exp=0;
					else if (exp==0x7fffffff) exp=0xff;
					else exp-=0x3fffff80;
					*fp=r->dat[0]&(1<<31);
					*fp|=(r->dat[1]>>8)&0x007fffff;
					if (r->dat[1]&0xff || r->dat[2]) err=CVERR_CUT;
					if (exp>0xff && exp<0x80000000) { err=CVERR_EXP; exp=0xff; }
					if (exp>=0x80000000) { err=CVERR_EXP; *fp=0; }
					else *fp|=exp<<23;
				} else *fp=0;
			} break;
			case 8: // double
			{
				ULONG *fp=(ULONG *)f;
				
				if (r->dat[0] || r->dat[1] || r->dat[2])
				{
					ULONG exp=r->dat[0]&0x7fffffff;
					if (exp==0) exp=0;
					else if (exp==0x7fffffff) exp=0x7ff;
					else exp-=0x3ffffc00;
					fp[0]=r->dat[0]&(1<<31);
					fp[0]|=(r->dat[1]>>11)&0x000fffff;
					fp[1]=((r->dat[1]&0x000007ff)<<21)|((r->dat[2]&0xfffff800)>>11);
					if (r->dat[2]&0x7ff) err=CVERR_CUT;
					if (exp>0x7ff && exp<0x80000000) { err=CVERR_EXP; exp=0x7ff; }
					if (exp>=0x80000000) { err=CVERR_EXP; fp[0]=fp[1]=0; }
					else fp[0]|=exp<<20;
				} else fp[0]=fp[1]=0;
			} break;
			case 12: // extend (nicht implementiert)
			{
				ULONG *fp=(ULONG *)f;
				fp[0]=r->dat[0];			
				fp[1]=r->dat[1];
				fp[2]=r->dat[2];			
			} break;
			default: err=CVERR_PREC;
		}
	} else err=CVERR_NULL;
	return(err);
}
