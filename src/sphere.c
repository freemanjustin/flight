#include "slicer.h"

/*
   Create a sphere centered at c, with radius r, and precision n
   Draw a point for zero radius spheres
   Use CCW facet ordering
   "method" is 0 for quads, 1 for triangles
      (quads look nicer in wireframe mode)
   Partial spheres can be created using theta1->theta2, phi1->phi2
   in radians 0 < theta < 2pi, -pi/2 < phi < pi/2
   Slight modified from the source code taken from http://astronomy.swin.edu.au/~pbourke/opengl/sphere/
*/


void DrawSphere(e *E, int n, double xc, double yc, double zc, double r)
{
	int i,j;
	//int n;
	double t1,t2,t3;
	float e[3],p[3];
   
	// set sphere resolution (n) based on zoom level
	/*
	if(E->viewer[2] >= 2.0)
		n = 80;
	else if( (E->viewer[2] < 2.0) && (E->viewer[2] >= 1.5))
		n = 100;
	else
		n = 150;
	*/	
	
	// set the resolution to a constant for now:
	//n=100;

	for (j=0;j<n/2;j++) {
		t1 = (j*PI / (n/2)) - (PI/2);
		t2 = ((j+1)*PI / (n/2)) - (PI/2);
 
		glBegin(GL_TRIANGLE_STRIP);
		//glBegin(GL_QUAD_STRIP);
		for (i=0;i<=n;i++) {
			t3 = 2*i*PI / n - (PI/2);
 
			e[0] = cos(t1) * cos(t3);
			e[1] = sin(t1);
			e[2] = cos(t1) * sin(t3);
			p[0] = xc + r * e[0];
			p[1] = yc + r * e[1];
			p[2] = zc + r * e[2];
			glNormal3f(e[0],e[1],e[2]);
	 
			// j - swapped the x texture coordinates so that the 
			// texture maps show up properly
			// this commented line is the original texCoord
			//glTexCoord2f(i/(double)n,2*j/(double)n);
			glTexCoord2f( (n-i)/(double)n, 2*j/(double)n);
			glVertex3f(p[0],p[1],p[2]);

			e[0] = cos(t2) * cos(t3);
			e[1] = sin(t2);
			e[2] = cos(t2) * sin(t3);
			p[0] = xc + r * e[0];
			p[1] = yc + r * e[1];
			p[2] = zc + r * e[2];
			glNormal3f(e[0],e[1],e[2]);
	 
			// j - swapped the x texture coordinates so that the 
			// texture maps show up properly
			// this commented line is the original texCoord
			//glTexCoord2f(i/(double)n,2*(j+1)/(double)n);
			glTexCoord2f( (n-i)/(double)n,2*(j+1)/(double)n);
			glVertex3f(p[0],p[1],p[2]);
		}
		glEnd();
	}
}

void DrawSphereII(e *E, double xc, double yc, double zc, double r)
{
	int i,j,n;
	double t1,t2,t3;
	float e[3],p[3];
   
   
	// set sphere resolution (n) based on zoom level
	/*
	if(E->viewer[2] >= 2.0)
		n = 80;
	else if( (E->viewer[2] < 2.0) && (E->viewer[2] >= 1.5))
		n = 100;
	else
		n = 150;
	*/	
	
	// set the resolution to a constant for now:
	n=100;



	for (j=0;j<n/4;j++) {
		t1 = (j*PI / (n/2)) - (PI/2);
		t2 = ((j+1)*PI / (n/2)) - (PI/2);
 
		glBegin(GL_TRIANGLE_STRIP);
 
		for (i=0;i<=n;i++) {
			t3 = 2*i*PI / n - (PI/2);
 
			e[0] = cos(t1) * cos(t3);
			e[1] = sin(t1);
			e[2] = cos(t1) * sin(t3);
			p[0] = xc + r * e[0];
			p[1] = yc + r * e[1];
			p[2] = zc + r * e[2];
			glNormal3f(e[0],e[1],e[2]);
	 
			// j - swapped the x texture coordinates so that the 
			// texture maps show up properly
			// this commented line is the original texCoord
			//glTexCoord2f(i/(double)n,2*j/(double)n);
			glTexCoord2f( (n-i)/(double)n, 2*j/(double)n);
			glVertex3f(p[0],p[1],p[2]);

			e[0] = cos(t2) * cos(t3);
			e[1] = sin(t2);
			e[2] = cos(t2) * sin(t3);
			p[0] = xc + r * e[0];
			p[1] = yc + r * e[1];
			p[2] = zc + r * e[2];
			glNormal3f(e[0],e[1],e[2]);
	 
			// j - swapped the x texture coordinates so that the 
			// texture maps show up properly
			// this commented line is the original texCoord
			//glTexCoord2f(i/(double)n,2*(j+1)/(double)n);
			glTexCoord2f( (n-i)/(double)n,2*(j+1)/(double)n);
			glVertex3f(p[0],p[1],p[2]);
		}
		glEnd();
	}
}

void DrawSphere_invert_normals(e *E, int n, double xc, double yc, double zc, double r)
{
	int i,j;
	//int n;
	double t1,t2,t3;
	float e[3],p[3];
	
	// set sphere resolution (n) based on zoom level
	/*
	 if(E->viewer[2] >= 2.0)
	 n = 80;
	 else if( (E->viewer[2] < 2.0) && (E->viewer[2] >= 1.5))
	 n = 100;
	 else
	 n = 150;
	 */	
	
	// set the resolution to a constant for now:
	//n=100;
	
	
	for (j=0;j<n/2;j++) {
		t1 = (j*PI / (n/2)) - (PI/2);
		t2 = ((j+1)*PI / (n/2)) - (PI/2);
		
		glBegin(GL_TRIANGLE_STRIP);
		//glBegin(GL_QUAD_STRIP);
		for (i=0;i<=n;i++) {
			t3 = 2*i*PI / n - (PI/2);
			
			e[0] = cos(t1) * cos(t3);
			e[1] = sin(t1);
			e[2] = cos(t1) * sin(t3);
			p[0] = xc + r * e[0];
			p[1] = yc + r * e[1];
			p[2] = zc + r * e[2];
			glNormal3f(-e[0],-e[1],-e[2]);
			
			// j - swapped the x texture coordinates so that the 
			// texture maps show up properly
			// this commented line is the original texCoord
			//glTexCoord2f(i/(double)n,2*j/(double)n);
			glTexCoord2f( (n-i)/(double)n, 2*j/(double)n);
			glVertex3f(p[0],p[1],p[2]);
			
			e[0] = cos(t2) * cos(t3);
			e[1] = sin(t2);
			e[2] = cos(t2) * sin(t3);
			p[0] = xc + r * e[0];
			p[1] = yc + r * e[1];
			p[2] = zc + r * e[2];
			glNormal3f(-e[0],-e[1],-e[2]);
			
			// j - swapped the x texture coordinates so that the 
			// texture maps show up properly
			// this commented line is the original texCoord
			//glTexCoord2f(i/(double)n,2*(j+1)/(double)n);
			glTexCoord2f( (n-i)/(double)n,2*(j+1)/(double)n);
			glVertex3f(p[0],p[1],p[2]);
		}
		glEnd();
	}
}



