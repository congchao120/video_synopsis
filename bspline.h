#ifndef _BSPLINE_H
#define _BSPLINE_H

void knot(int n, int c, int x[]);
void basis(int c,float t, int npts,int x[],float n[]);

void bspline(int npts,int k,int p1,float b[],float p[]);


#endif