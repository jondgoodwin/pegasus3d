/** Matrix operations
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/


#include "pegasus3d.h"
#include <stdio.h>
#include "xyzmath.h"

/* Pre-calculated constants for fast approximation of spherical linear interpolation */
Afloat quatSlerpU[8]={
	0.333333333333333f, 0.100000000000000f, 0.047619047619048f, 0.027777777777778f,
	0.018181818181818f, 0.012820512820513f, 0.009523809523810f, 0.013978731275863f};
Afloat quatSlerpV[8]={
	0.333333333333333f, 0.400000000000000f, 0.428571428571429f, 0.444444444444444f,
	0.454545454545455f, 0.461538461538462f, 0.466666666666667f, 0.894638801655200f};

/** Calculate and return Quat value spherically/linearly interpolated between two Quat values by a scalar from 0 to 1 */
/* Faster slerp approimation algorithm from David Eberly, adapted from Cesium.js, accurate to 10 e -6. */
void quatSlerp(Quat *q, Quat *q1, Quat *q2, Afloat scalar) {
	
	Afloat dot = quatDot(q1, q2);
	Afloat sign;
	if (dot>=0)
		sign = 1.0f;
	else {
		sign = -1.0f;
		dot = -dot;
	}

	Afloat dotm1 = dot - 1.0f;
	Afloat scalarOpp = 1.0f - scalar;
	Afloat sqrScalar = scalar * scalar;
	Afloat sqrScalarOpp = scalarOpp * scalarOpp;

	Afloat bT[8], bD[8];
	for (int i = 7; i>=0; --i) {
		bT[i] = (quatSlerpU[i] * sqrScalar - quatSlerpV[i]) * dotm1;
		bD[i] = (quatSlerpU[i] * sqrScalarOpp - quatSlerpV[i]) * dotm1;
	}

	Afloat cT = sign * scalar * (
		1.0f + bT[0] * (1.0f + bT[1] * (1.0f + bT[2] * (1.0f + bT[3] * (
	    1.0f + bT[4] * (1.0f + bT[5] * (1.0f + bT[6] * (1.0f + bT[7]))))))));
    Afloat cD = scalarOpp * (
        1.0f + bD[0] * (1.0f + bD[1] * (1.0f + bD[2] * (1.0f + bD[3] * (
        1.0f + bD[4] * (1.0f + bD[5] * (1.0f + bD[6] * (1.0f + bD[7]))))))));

	q->x = cD*q1->x + cT*q2->x;
	q->y = cD*q1->y + cT*q2->y;
	q->z = cD*q1->z + cT*q2->z;
	q->w = cD*q1->w + cT*q2->w;
}

/** Copy numbers from mat to to mat */
void mat4Set(Mat4 *tomat, Mat4 *frommat) {
	memcpy((void*)tomat, (void*)frommat, 16*sizeof(float));
}

/** Translation matrix */
void mat4SetPos(Mat4 *mat, Xyz *xyz) {
	(*mat)[12]=xyz->x;
	(*mat)[13]=xyz->y;
	(*mat)[14]=xyz->z;
}

/** Multiply two matrices m1 and m2. Put result in md. */
void mat4Mult(Mat4 *md, Mat4 *m1, Mat4 *m2) {
	Mat4 tempmat;
	if (md==m1) {
		memcpy(tempmat, md, sizeof(Mat4));
		m1 = &tempmat;
	}
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

/** Multiple a matrix by a vector (whose w is presumed to be 1) */
void mat4MultVec(Xyz *newxyz, Mat4 *mat, Xyz *xyz) {
	newxyz->x = (*mat)[0]*xyz->x + (*mat)[4]*xyz->y + (*mat)[8]*xyz->z + (*mat)[12];
	newxyz->y = (*mat)[1]*xyz->x + (*mat)[5]*xyz->y + (*mat)[9]*xyz->z + (*mat)[13];
	newxyz->z = (*mat)[2]*xyz->x + (*mat)[6]*xyz->y + (*mat)[10]*xyz->z + (*mat)[14];
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

/** Scale existing matrix by xyz scaling (can use after creating rotation) */
void mat4Scale(Mat4 *mat, Xyz *s) {
	(*mat)[0] *= s->x;
	(*mat)[1] *= s->x;
	(*mat)[2] *= s->x;

	(*mat)[4] *= s->y;
	(*mat)[5] *= s->y;
	(*mat)[6] *= s->y;

	(*mat)[8] *= s->z;
	(*mat)[9] *= s->z;
	(*mat)[10] *= s->z;
}

/** Create matrix based on a quaternion's rotation */
void mat4Quat(Mat4 *mat, Quat *q) {
	float x = q->x, y = q->y, z = q->z, w = q->w;
	float x2 = x + x, y2 = y + y, z2 = z + z;
	float xx = x * x2, xy = x * y2, xz = x * z2;
	float yy = y * y2, yz = y * z2, zz = z * z2;
	float wx = w * x2, wy = w * y2, wz = w * z2;

	(*mat)[0] = 1.f - ( yy + zz );
	(*mat)[1] = xy + wz;
	(*mat)[2] = xz - wy;

	(*mat)[4] = xy - wz;
	(*mat)[5] = 1.f - ( xx + zz );
	(*mat)[6] = yz + wx;

	(*mat)[8] = xz + wy;
	(*mat)[9] = yz - wx;
	(*mat)[10] = 1.f - ( xx + yy );

	(*mat)[12] = (*mat)[13] = (*mat)[14] = 0.f;
	(*mat)[3] = (*mat)[7] = (*mat)[11] = 0.f;
	(*mat)[15] = 1.f;
}

/** Create lookat matrix using location, center of what we look at, and the up direction.
	This one translates from local/eye to parent/world coordinates */
void mat4Lookat(Mat4 *mat, Xyz *loc, Xyz *at, Xyz *up) {
	Xyz disp, ndisp, third, nthird, nup;

	disp.x=at->x-loc->x;
	disp.y=at->y-loc->y;
	disp.z=at->z-loc->z;
	xyzNorm(&ndisp, &disp);
	
	xyzCross(&third, &ndisp, up);
	xyzNorm(&nthird, &third);
	xyzCross(&nup, &nthird, &ndisp);

	(*mat)[0]  = nthird.x; // New x-axis
	(*mat)[1]  = nthird.y;
	(*mat)[2]  = nthird.z;
	(*mat)[12]  = loc->x;
	(*mat)[4]  = nup.x; // New y-axis
	(*mat)[5]  = nup.y;
	(*mat)[6]  = nup.z;
	(*mat)[13]  = loc->y;
	(*mat)[8]  = -ndisp.x; // New x-axis
	(*mat)[9]  = -ndisp.y;
	(*mat)[10] = -ndisp.z;
	(*mat)[14] = loc->z;
	(*mat)[3] = 0.0;
	(*mat)[7] = 0.0;
	(*mat)[11] = 0.0;
	(*mat)[15] = 1.0;
}

/** Create lookat matrix using eye, center of what we look at, and the up direction.
	This one (used by cameras) translates from world to eye/camera coordinates */
void mat4InverseLookat(Mat4 *mat, Xyz *eye, Xyz *center, Xyz *up) {
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
	(*mat)[8]  = nthird.z;
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

/** Calculate a rotation view matrix using euler angles.
	Equivalent to Translate*Rotz*Roty*Rotz*Scale */
void mat4Rotate(Mat4 *mat, Xyz *pos, Xyz *rot, Xyz *scale) {
	(*mat)[ 0] = scale->x * cos(-rot->y)*cos(rot->z);
	(*mat)[ 1] = scale->x * cos(-rot->y)*sin(rot->z);
	(*mat)[ 2] = scale->x * sin(-rot->y);
	(*mat)[ 3] = 0.0f;
	(*mat)[ 4] = scale->y * (-cos(rot->x)*sin(rot->z) - sin(rot->x)*sin(-rot->y)*cos(rot->z));
	(*mat)[ 5] = scale->y * (cos(rot->x)*cos(rot->z) - sin(rot->x)*sin(-rot->y)*sin(rot->z));
	(*mat)[ 6] = scale->y * sin(rot->x)*cos(-rot->y);
	(*mat)[ 7] = 0.0f;
	(*mat)[ 8] = scale->z * (sin(rot->x)*sin(rot->z) - cos(rot->x)*sin(-rot->y)*cos(rot->z));
	(*mat)[ 9] = scale->z * (-sin(rot->x)*cos(rot->z) - cos(rot->x)*sin(-rot->y)*sin(rot->z));
	(*mat)[10] = scale->z * cos(rot->x)*cos(-rot->y);
	(*mat)[11] = 0.0f;
	(*mat)[12] = pos->x;
	(*mat)[13] = pos->y;
	(*mat)[14] = pos->z;
	(*mat)[15] = 1.0f;
}

/** Invert a matrix, for use in calculating camera's matrix.
	Note: this is a simplified solution which assumes bottom row is always 0,0,0,1*/
void mat4Inverse(Mat4 *tmat, Mat4 *fmat) {
	Mat4 tempmat;
	if (fmat==tmat) {
		memcpy(tempmat, fmat, sizeof(Mat4));
		fmat = &tempmat;
	}

	float det = (*fmat)[0]*(*fmat)[5]*(*fmat)[10] 
		+ (*fmat)[4]*(*fmat)[9]*(*fmat)[2] 
		+ (*fmat)[4]*(*fmat)[1]*(*fmat)[6] 
		- (*fmat)[0]*(*fmat)[9]*(*fmat)[6] 
		- (*fmat)[4]*(*fmat)[5]*(*fmat)[10] 
		- (*fmat)[8]*(*fmat)[5]*(*fmat)[2];

	(*tmat)[0]  = ((*fmat)[ 5]*(*fmat)[10] - (*fmat)[ 9]*(*fmat)[ 6])/det;
	(*tmat)[4]  = ((*fmat)[ 8]*(*fmat)[ 6] - (*fmat)[ 4]*(*fmat)[10])/det;
	(*tmat)[8]  = ((*fmat)[ 4]*(*fmat)[ 9] - (*fmat)[ 8]*(*fmat)[ 5])/det;
	(*tmat)[1]  = ((*fmat)[ 9]*(*fmat)[ 2] - (*fmat)[ 1]*(*fmat)[10])/det;
	(*tmat)[5]  = ((*fmat)[ 0]*(*fmat)[10] - (*fmat)[ 8]*(*fmat)[ 2])/det;
	(*tmat)[9]  = ((*fmat)[ 8]*(*fmat)[ 1] - (*fmat)[ 0]*(*fmat)[ 9])/det;
	(*tmat)[2]  = ((*fmat)[ 1]*(*fmat)[ 6] - (*fmat)[ 5]*(*fmat)[ 2])/det;
	(*tmat)[6]  = ((*fmat)[ 4]*(*fmat)[ 2] - (*fmat)[ 0]*(*fmat)[ 6])/det;
	(*tmat)[10] = ((*fmat)[ 0]*(*fmat)[ 5] - (*fmat)[ 4]*(*fmat)[ 1])/det;

	(*tmat)[12] = ((*fmat)[4]*(*fmat)[13]*(*fmat)[10] 
		+ (*fmat)[8]*(*fmat)[5]*(*fmat)[14] 
		+ (*fmat)[12]*(*fmat)[9]*(*fmat)[6] 
		- (*fmat)[4]*(*fmat)[9]*(*fmat)[14] 
		- (*fmat)[8]*(*fmat)[13]*(*fmat)[6] 
		- (*fmat)[12]*(*fmat)[5]*(*fmat)[10])/det;
	(*tmat)[13] = ((*fmat)[0]*(*fmat)[9]*(*fmat)[14] 
		+ (*fmat)[8]*(*fmat)[9]*(*fmat)[2] 
		+ (*fmat)[12]*(*fmat)[1]*(*fmat)[12] 
		- (*fmat)[0]*(*fmat)[13]*(*fmat)[10] 
		- (*fmat)[8]*(*fmat)[1]*(*fmat)[14] 
		- (*fmat)[12]*(*fmat)[9]*(*fmat)[2])/det;
	(*tmat)[14] = ((*fmat)[0]*(*fmat)[13]*(*fmat)[6] 
		+ (*fmat)[4]*(*fmat)[1]*(*fmat)[14] 
		+ (*fmat)[12]*(*fmat)[5]*(*fmat)[2] 
		- (*fmat)[0]*(*fmat)[5]*(*fmat)[14] 
		- (*fmat)[4]*(*fmat)[13]*(*fmat)[2] 
		- (*fmat)[12]*(*fmat)[1]*(*fmat)[6])/det;

	(*tmat)[3] = 0.0f;
	(*tmat)[7] = 0.0f;
	(*tmat)[11] = 0.0f;
	(*tmat)[15] = 1.0f;
}

/** Print out a matrix, for diagnostics */
void mat4Print(Mat4 *mat, const char *matnm) {
	printf("%s %p\n", matnm, mat);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[0], (*mat)[4], (*mat)[8], (*mat)[12]);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[1], (*mat)[5], (*mat)[9], (*mat)[13]);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[2], (*mat)[6], (*mat)[10], (*mat)[14]);
	printf("  %.5f %.5f %.5f %.5f\n", (*mat)[3], (*mat)[7], (*mat)[11], (*mat)[15]);
}

