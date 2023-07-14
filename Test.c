#include <stdio.h>
#include <math.h>
#include <exec/types.h>
#include <user/user.h>

main()
{
	double a,exp;
	float f,fcf,fcd;
	double d,dcf,dcd;
	real fr,dr;
	int err[6];
	
	printf("Geben Sie eine Zahl ein: \n");
	scanf("%lf",&a);
	printf("Geben Sie den Exponenten ein: \n");
	scanf("%lf",&exp);
	
	d=pow(a,exp);
	
	f=(float)d;
	printf("->Wert: %lg\n",d);
	
	printf("Bitmuster float: %p, double: %p-%p\n\n",((ULONG *)&f)[0],((ULONG *)&d)[0],((ULONG *)&d)[1]);
	
	err[0]=toreal(&fr,&f);
	err[1]=toreal(&dr,&d);
	
	printf("Bitmuster real von float : %p-%p-%p\n",((ULONG *)&fr)[0],((ULONG *)&fr)[1],((ULONG *)&fr)[2]);
	printf("Bitmuster real von double: %p-%p-%p\n\n\n",((ULONG *)&dr)[0],((ULONG *)&dr)[1],((ULONG *)&dr)[2]);
	
	err[2]=byreal(&fcf,&fr);
	err[3]=byreal(&fcd,&dr);

	err[4]=byreal(&dcf,&fr);
	err[5]=byreal(&dcd,&dr);

	printf("Bitmuster float von real(float) : %p, Wert: %hg\n",((ULONG *)&fcf)[0],fcf);
	printf("Bitmuster float von real(double): %p, Wert: %hg\n\n",((ULONG *)&fcd)[0],fcd);

	printf("Bitmuster double von real(float) : %p-%p, Wert: %lg\n",((ULONG *)&dcf)[0],((ULONG *)&dcf)[1],dcf);
	printf("Bitmuster double von real(double): %p-%p, Wert: %lg\n\n\n",((ULONG *)&dcd)[0],((ULONG *)&dcd)[1],dcd);
	
	printf("Fehlercodes: %d,%d,%d,%d,%d,%d\n",err[0],err[1],err[2],err[3],err[4],err[5]);
}
