#include "bspline.h"

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
/*
Subroutine to generate a B-spline open knot vector with multiplicity
equal to the order at the ends.

c            = order of the basis function
n            = the number of defining polygon vertices
nplus2       = index of x() for the first occurence of the maximum knot vector value
nplusc       = maximum value of the knot vector -- $n + c$
x()          = array containing the knot vector
*/
void knot(int n, int c, int x[])
{
	int nplusc,nplus2,i;

	nplusc = n + c;
	nplus2 = n + 2;

	x[1] = 0;
	for (i = 2; i <= nplusc; i++){
		if ( (i > c) && (i < nplus2) )
			x[i] = x[i-1] + 1;
		else
			x[i] = x[i-1];
	}
}
/*  Subroutine to generate B-spline basis functions for open knot vectors

C code for An Introduction to NURBS
by David F. Rogers. Copyright (C) 2000 David F. Rogers,
All rights reserved.

Name: basis.c
Language: C
Subroutines called: none
Book reference: p. 279

c        = order of the B-spline basis function
d        = first term of the basis function recursion relation
e        = second term of the basis function recursion relation
npts     = number of defining polygon vertices
n[]      = array containing the basis functions
n[1] contains the basis function associated with B1 etc.
nplusc   = constant -- npts + c -- maximum number of knot values
t        = parameter value
temp[]   = temporary array
x[]      = knot vector
*/  

void basis(int c,float t, int npts,int x[],float n[])
{
	int nplusc;
	int i,k;
	float d,e;
	float temp[36];

	nplusc = npts + c;

	/* calculate the first order basis functions n[i][1]    */

	for (i = 1; i<= nplusc-1; i++){
		if (( t >= x[i]) && (t < x[i+1]))
			temp[i] = 1;
		else
			temp[i] = 0;
	}

	/* calculate the higher order basis functions */

	for (k = 2; k <= c; k++){
		for (i = 1; i <= nplusc-k; i++){
			if (temp[i] != 0)/* if the lower order basis function is zero skip the                         calculation */
				d = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
			else
				d = 0;

			if (temp[i+1] != 0)     /* if the lower order basis function is zero skip the calculation  */
				e = ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
			else
				e = 0;

			temp[i] = d + e;
		}
	}

	if (t == (float)x[nplusc]){     /*    pick up last point    */
		temp[npts] = 1;
	}

	/* put in n array   */

	for (i = 1; i <= npts; i++) {
		n[i] = temp[i];
	}
}

/*Subroutine to generate a B-spline curve using an uniform open knot vector

C code for An Introduction to NURBS
by David F. Rogers. Copyright (C) 2000 David F. Rogers,
All rights reserved.

Name: bspline.c
Language: C
Subroutines called: knot.c, basis.c, fmtmul.c
Book reference: Section 3.5, Ex. 3.4, Alg. p. 281

b[]        = array containing the defining polygon vertices
b[1] contains the x-component of the vertex
b[2] contains the y-component of the vertex
b[3] contains the z-component of the vertex
k           = order of the \bsp basis function
nbasis      = array containing the basis functions for a single value of t
nplusc      = number of knot values
npts        = number of defining polygon vertices
p[,]        = array containing the curve points
p[1] contains the x-component of the point
p[2] contains the y-component of the point
p[3] contains the z-component of the point
p1          = number of points to be calculated on the curve
t           = parameter value 0 <= t <= 1
x[]         = array containing the knot vector
*/

void bspline(int npts,int k,int p1,float b[],float p[])
{
	int i,j,icount,jcount;
	int i1;
	int x[30];      /* allows for 20 data points with basis function of order 5 */
	int nplusc;

	float step;
	float t;
	float nbasis[20];
	float temp;

	nplusc = npts + k;

	/*  zero and re-dimension the knot vector and the basis array */

	for(i = 0; i <= npts; i++){
		nbasis[i] = 0.;
	}

	for(i = 0; i <= nplusc; i++){
		x[i] = 0.;
	}

	/* generate the uniform open knot vector */

	knot(npts,k,x);
	icount = 0;

	/*calculate the points on the bspline curve */

	t = 0;
	step = ((float)x[nplusc])/((float)(p1-1));

	for (i1 = 1; i1<= p1; i1++){

		if ((float)x[nplusc] - t < 5e-6){
			t = (float)x[nplusc];
		}

		basis(k,t,npts,x,nbasis);      /* generate the basis function for this value of t */
		for (j = 1; j <= 3; j++){      /* generate a point on the curve */
			jcount = j;
			p[icount+j] = 0.;

			for (i = 1; i <= npts; i++){ /* Do local matrix multiplication */
				temp = nbasis[i]*b[jcount];
				p[icount + j] = p[icount + j] + temp;
				jcount = jcount + 3;
			}
		}

		icount = icount + 3;
		t = t + step;
	}
}

