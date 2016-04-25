#include "slicer.h"


void lons_wrapped_orig(double *lons, int n, double *lons_uw) {
	
	int	i;
    double offset=0, diff;
	//lon_str = "";   
	
    lons_uw[0] = lons[0];
    for (i=0;i<n-1;i++) {
		diff = lons[i] - lons[i+1];
		
		if (fabs(diff) > 180) {	    
			if (lons[i] < 0) {
				offset = offset - 360;
			} 
			else {
				offset = offset + 360;
			}
		}
		lons_uw[i+1] = lons[i+1] + offset;
		//lon_str += round(lons[i],1)+","+offset+"\n";
    }
    //alert(lon_str);
	
}

void lons_wrapped(double *lons, int n, double *lons_uw) {
	
	int	i;
    double offset=0, diff;
	//lon_str = "";   
	
    lons_uw[0] = lons[0];
    for (i=0;i<n-1;i++) {
		diff = lons[i] - lons[i+1];
		
		if (fabs(diff) > 180) {	    
			if (lons[i] < 0) {
				offset = offset - 180;
			} 
			else {
				offset = offset + 180;
			}
		}
		lons_uw[i+1] = lons[i+1] + offset;
		//lon_str += round(lons[i],1)+","+offset+"\n";
    }
    //alert(lon_str);
	
}




void spline(double *x, double *y, int n, double yp1, double ypn, double *y2) {

    double	*u; 
    int		i, k;
    double	p, qn, sig, un;
	
	u = malloc(n*sizeof(double));
		
    if (yp1 > 0.99e30) {
        y2[0] = u[0] = 0.0;
    } 
    else {
        y2[0] = -0.5;
        u[0] = (3.0/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp1);
    }
		
    for(i = 1; i < n-1; i++){
        sig = (x[i] - x[i-1])/(x[i+1] - x[i-1]);
        p = sig * y2[i-1] + 2.0;
        y2[i] = (sig - 1.0)/p;
        u[i] = (y[i+1] - y[i])/(x[i+1] - x[i]) - (y[i] - y[i-1])/(x[i] - x[i-1]);
        u[i] = (6.0*u[i]/(x[i+1] - x[i-1]) - sig*u[i-1])/p;
		
    }
	
    if (ypn > 0.99e30) {
        qn = un = 0.0;
    } 
    else {
        qn = 0.5;
        un = (3.0/(x[n-1] - x[n-2]))*(ypn - (y[n-1] - y[n-2])/(x[n-1] - x[n-2]));
    }
	
    y2[n-1] = (un - qn*u[n-2])/(qn*y2[n-2] + 1.0);
	
    for (k = n-2; k >= 0; k--){
        y2[k] = y2[k] * y2[k+1] + u[k];
    }
	
	free(u);
	
}


double splint(double *xa, double *ya, double *y2a, int n, double x) {
	int klo,khi,k;
	double h,b,a;
	static int pklo=0,pkhi=1;
	
	if (xa[pklo] <= x && xa[pkhi] > x) {
		klo = pklo;
		khi = pkhi;
	} else {
		klo = 0;
		khi = n - 1;
		while (khi - klo > 1) {
			k = (khi + klo) >> 1;
			if (xa[k] > x) {
				khi = k;
			} else {
				klo = k;
			}
		}
	}
	
	h = xa[khi] - xa[klo];
	if (h == 0) {
		fprintf(stderr,"Bad xa input to function splint()\n");
		exit(1);
	}
	
	a = (xa[khi] - x)/h;
	b = (x - xa[klo])/h;
	return a*ya[klo] + b*ya[khi] + ((a*a*a - a)*y2a[klo] + (b*b*b - b)*y2a[khi])*(h*h)/6.0;
}

