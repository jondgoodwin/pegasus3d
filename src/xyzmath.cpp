/** Matrix operations
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/


#include "pegasus3d.h"
#include <stdio.h>
#include "xyzmath.h"

/** Translation matrix */
void mat4Pos(Mat4 *mat, GLfloat x, GLfloat y, GLfloat z) {
	mat4Identity(mat);
	(*mat)[12]=x;
	(*mat)[13]=y;
	(*mat)[14]=z;
}

/** Multiply two matrices m1 and m2. Put result in md. */
void mat4Mult(Mat4 *md, Mat4 *m1, Mat4 *m2) {
	(*md)[0] = (*m1)[0]*(*m2)[0] + (*m1)[4]*(*m2)[1] + (*m1)[ 8]*(*m2)[2] + (*m1)[12]*(*m2)[3];
	(*md)[1] = (*m1)[1]*(*m2)[0] + (*m1)[5]*(*m2)[1] + (*m1)[ 9]*(*m2)[2] + (*m1)[13]*(*m2)[3];
	(*md)[2] = (*m1)[2]*(*m2)[0] + (*m1)[6]*(*m2)[1] + (*m1)[10]*(*m2)[2] + (*m1)[14]*(*m2)[3];
	(*md)[3] = (*m1)[3]*(*m2)[0] + (*m1)[7]*(*m2)[1] + (*m1)[11]*(*m2)[2] + (*m1)[15]*(*m2)[3];

	(*md)[4] = (*m1)[0]*(*m2)[4] + (*m1)[4]*(*m2)[5] + (*m1)[ 8]*(*m2)[6] + (*m1)[12]*(*m2)[7];
	(*md)[5] = (*m1)[1]*(*m2)[4] + (*m1)[5]*(*m2)[5] + (*m1)[ 9]*(*m2)[6] + (*m1)[13]*(*m2)[7];
	(*md)[6] = (*m1)[2]*(*m2)[4] + (*m1)[6]*(*m2)[5] + (*m1)[10]*(*m2)[6] + (*m1)[14]*(*m2)[7];
	(*md)[7] = (*m1)[3]*(*m2)[4] + (*m1)[7]*(*m2)[5] + (*m1)[11]*(*m2)[6] + (*m1)[15]*(*m2)[7];

	(*md)[ 8] = (*m1)[0]*(*m2)[8] + (*m1)[4]*(*m2)[9] + (*m1)[ 8]*(*m2)[10] + (*m1)[12]*(*m2)[11];
	(*md)[ 9] = (*m1)[1]*(*m2)[8] + (*m1)[5]*(*m2)[9] + (*m1)[ 9]*(*m2)[10] + (*m1)[13]*(*m2)[11];
	(*md)[10] = (*m1)[2]*(*m2)[8] + (*m1)[6]*(*m2)[9] + (*m1)[10]*(*m2)[10] + (*m1)[14]*(*m2)[11];
	(*md)[11] = (*m1)[3]*(*m2)[8] + (*m1)[7]*(*m2)[9] + (*m1)[11]*(*m2)[10] + (*m1)[15]*(*m2)[11];

	(*md)[12] = (*m1)[0]*(*m2)[12] + (*m1)[4]*(*m2)[13] + (*m1)[ 8]*(*m2)[14] + (*m1)[12]*(*m2)[15];
	(*md)[13] = (*m1)[1]*(*m2)[12] + (*m1)[5]*(*m2)[13] + (*m1)[ 9]*(*m2)[14] + (*m1)[13]*(*m2)[15];
	(*md)[14] = (*m1)[2]*(*m2)[12] + (*m1)[6]*(*m2)[13] + (*m1)[10]*(*m2)[14] + (*m1)[14]*(*m2)[15];
	(*md)[15] = (*m1)[3]*(*m2)[12] + (*m1)[7]*(*m2)[13] + (*m1)[11]*(*m2)[14] + (*m1)[15]*(*m2)[15];
}

/** Perspective Frustum matrix */
void mat4Perspective(Mat4 *mat, GLfloat fov, GLfloat near, GLfloat far, GLfloat aspratio) {
	GLfloat tanhalffov = tanf(fov * (GLfloat)M_PI / (GLfloat)360.0);
	(*mat)[1] = (*mat)[2] = (*mat)[3] = 
	(*mat)[4] = (*mat)[6] = (*mat)[7] = 
	(*mat)[8] = (*mat)[9] = 
	(*mat)[12] = (*mat)[13] = (*mat)[15] = (GLfloat)0.0;

	(*mat)[0] = (GLfloat)1.0/(tanhalffov * aspratio);
	(*mat)[5] = (GLfloat)1.0/tanhalffov;
	(*mat)[10] = (far+near)/(near-far);
	(*mat)[11] = -1.0;
	(*mat)[14] = (GLfloat)2.0*far*near/(near-far); 
}

/** Perspective View matrix */
void mat4HeightPerspective(Mat4 *mat, GLfloat height, GLfloat near, GLfloat far, GLfloat aspratio) {
	mat4Identity(mat);
	(*mat)[0] = near/(height * aspratio);
	(*mat)[5] = near/height;
	(*mat)[10] = (far+near)/(near-far);
	(*mat)[11] = -1.0;
	(*mat)[14] = (GLfloat)2.0*far*near/(near-far);
	(*mat)[15] = 0.0;
}

/** Orthogonal View matrix */
void mat4Ortho(Mat4 *mat, GLfloat height, GLfloat near, GLfloat far, GLfloat aspratio) {
	mat4Identity(mat);
	(*mat)[0] = (GLfloat)1.0/(height * aspratio);
	(*mat)[5] = (GLfloat)1.0/height;
	(*mat)[10] = -(GLfloat)2.0/(near-far);
	(*mat)[14] = (far+near)/(near-far);
}

void mat4Lookat(Mat4 *mat, Xyz *eye, Xyz *center, Xyz *up) {
	Xyz disp, ndisp, third, nthird, nup;

	disp.x=center->x-eye->x;
	disp.y=center->y-eye->y;
	disp.z=center->z-eye->z;
	xyzNorm(&ndisp, &disp);
	
	xyzCross(&third, &ndisp, up);
	xyzNorm(&nthird, &third);
	xyzCross(&nup, &nthird, &ndisp);

	(*mat)[0]  = nthird.x; // New x-axis
	(*mat)[4]  = nthird.y;
	(*mat)[8]  = nthird.y;
	(*mat)[12]  = -xyzDot(&nthird, eye);
	(*mat)[1]  = nup.x; // New y-axis
	(*mat)[5]  = nup.y;
	(*mat)[9]  = nup.z;
	(*mat)[13]  = -xyzDot(&nup, eye);
	(*mat)[2]  = -ndisp.x; // New x-axis
	(*mat)[6]  = -ndisp.y;
	(*mat)[10] = -ndisp.z;
	(*mat)[14] = xyzDot(&ndisp, eye);
	(*mat)[3] = 0.0;
	(*mat)[7] = 0.0;
	(*mat)[11] = 0.0;
	(*mat)[15] = 1.0;
}

/** Print out a matrix, for diagnostics */
void mat4Print(Mat4 *mat, const char *matnm) {
	printf("%s\n", matnm);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[0], (*mat)[4], (*mat)[8], (*mat)[12]);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[1], (*mat)[5], (*mat)[9], (*mat)[13]);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[2], (*mat)[6], (*mat)[10], (*mat)[14]);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[3], (*mat)[7], (*mat)[11], (*mat)[15]);
}

