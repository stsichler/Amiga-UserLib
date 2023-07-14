/* Routinen zur Vektorrechnung mit 3-Dimensionalen Vektoren--------------- */

/* verwenden den Datentyp "vector" */

#include <user/user.h>
#include <math.h>

/* ------------------------------------------------------------------------ */

vector *VAdd(v,v1,v2)				/* v=v1+v2 (Vektorsumme) */
vector *v,*v1,*v2;				/* RETURN: &v */
{
	v->x=v1->x+v2->x;
	v->y=v1->y+v2->y;
	v->z=v1->z+v2->z;
	return(v);
}

vector *VSub(v,v1,v2)				/* v=v1-v2 (Vektordifferenz) */
vector *v,*v1,*v2;				/* RETURN: &v */
{
	v->x=v1->x-v2->x;
	v->y=v1->y-v2->y;
	v->z=v1->z-v2->z;
	return(v);
}

double VSProd(v1,v2)				/* x=v1·v2 (Skalarprodukt) */
vector *v1,*v2;					/* RETURN: x */
{
	double x;
	x=v1->x*v2->x+v1->y*v2->y+v1->z*v2->z;
	return(x);
}

vector *VProd(v,v1,v2)			/* v=v1×v2 (Vektorprodukt) */
vector *v,*v1,*v2;				/* RETURN: &v */
{
	vector zv;
	zv.x=v1->y*v2->z-v1->z*v2->y;
	zv.y=v1->z*v2->x-v1->x*v2->z;
	zv.z=v1->x*v2->y-v1->y*v2->x;
	*v=zv;
	return(v);
}

double VAngle(v1,v2)				/* x=v1,v2 (Winkel zwischen v1 u. v2) */
vector *v1,*v2;					/* RETURN: x [0;pi] oder. -1.0 bei Fehler */
{
	double x,l1,l2;
	if (!(l1=VLen(v1))) return(-1.0);
	if (!(l2=VLen(v2))) return(-1.0);
	x=acos(VSProd(v1,v2)/l1/l2);
	return(x);
}

double VAngleN(v1,v2)				/* x=v1,v2 (Winkel zwischen v1 u. v2) */
vector *v1,*v2;					/* RETURN: x [0;pi] oder. -1.0 bei Fehler */
{								/* ACHTUNG: nur für normierte Vektoren */
	double x;
	x=acos(VSProd(v1,v2));
	return(x);
}

double VLen(v)						/* x=|v| (Länge des Vektors) */
vector *v;						/* RETURN: x */
{
	double x;
	x=sqrt(v->x*v->x+v->y*v->y+v->z*v->z); /* =sqrt(VSProd(v,v)) */
	return(x);
}

vector *VNorm(v)					/* v=v0 (Vektor v wird normiert) */
vector *v;						/* RETURN: &v */
{
	double l;
	if ((l=VLen(v))==0.0) return(v);
	return(VSDiv(v,l));
}

vector *VSMul(v,x)				/* v=v*x (skalare Multiplikation) */
vector *v;						/* RETURN: &v */
double x;
{
	v->x*=x;
	v->y*=x;
	v->z*=x;
	return(v);
}

vector *VSDiv(v,x)				/* v=v/x (skalare Division) */
vector *v;						/* RETURN: &v oder 0 bei x=0.0 */
double x;
{
	if (x==0.0) return(0);
	v->x/=x;
	v->y/=x;
	v->z/=x;
	return(v);
}

