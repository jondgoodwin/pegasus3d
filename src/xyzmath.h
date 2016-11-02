/** Spatial math: Xyz, Quaternion and Matrix operations
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#ifndef xyzmath_h
#define xyzmath_h 1

// GLEW Headers - for OpenGL GLfloat
#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <math.h>

/** Inline calculation of length of xyz vector */
#define xyzLen(xyz) \
	(sqrt((xyz)->x*(xyz)->x + (xyz)->y*(xyz)->y + (xyz)->z*(xyz)->z))

/** Return a new Xyz value that is the unit equivalent to self */
#define xyzNorm(newxyz, xyz) \
	{GLfloat len = xyzLen(xyz); \
	 if (len>0.0f) { \
		len = 1.0f/len; \
		(newxyz)->x = (xyz)->x*len; \
		(newxyz)->y = (xyz)->y*len; \
		(newxyz)->z = (xyz)->z*len;} \
	else {(newxyz)->x = 1.0f; (newxyz)->y = (newxyz)->z = 0.0f;}}

/** Return a new Xyz value that is the addition of xyz1 and xyz2 */
#define xyzAdd(newxyz, xyz1, xyz2) \
	{(newxyz)->x = (xyz1)->x + (xyz2)->x; \
	(newxyz)->y = (xyz1)->y + (xyz2)->y; \
	(newxyz)->z = (xyz1)->z + (xyz2)->z;}

/** Return a new Xyz value that is the subtraction of xyz1 and xyz2 */
#define xyzSub(newxyz, xyz1, xyz2) \
	{(newxyz)->x = (xyz1)->x - (xyz2)->x; \
	(newxyz)->y = (xyz1)->y - (xyz2)->y; \
	(newxyz)->z = (xyz1)->z - (xyz2)->z;}

/** Return a new Xyz value that is the cross product of two Xyz values */
#define xyzCross(newxyz, xyz1, xyz2) \
	{float x = (xyz1)->x; float y = (xyz1)->y; float z = (xyz1)->z; \
	(newxyz)->x = y*(xyz2)->z - z*(xyz2)->y; \
	(newxyz)->y = z*(xyz2)->x - x*(xyz2)->z; \
	(newxyz)->z = x*(xyz2)->y - y*(xyz2)->x;}

#define xyzDot(xyz1, xyz2) \
	((xyz1)->x*(xyz2)->x + (xyz1)->y*(xyz2)->y + (xyz1)->z*(xyz2)->z)

// ***********************************************************

#define quatDot(q1, q2) \
	((q1)->x*(q2)->x + (q1)->y*(q2)->y + (q1)->z*(q2)->z + (q1)->w*(q2)->w)
void quatSlerp(Quat *q, Quat *q1, Quat *q2, Afloat scalar);
// ***********************************************************

/** 4x4 floating point Matrix operations for right-hand orientation.
   The matrix's single-precision floating point numbers are stored in "column-first order", matching OpenGL.
     0   4   8  12
	 1   5   9  13
	 2   6  10  14
	 3   7  11  15
*/

/** Build an identity matrix */
#define mat4Identity(matrix) {\
	(*matrix)[1] = (*matrix)[2] = (*matrix)[3] = \
	(*matrix)[4] = (*matrix)[6] = (*matrix)[7] = \
	(*matrix)[8] = (*matrix)[9] = (*matrix)[11] = \
	(*matrix)[12] = (*matrix)[13] = (*matrix)[14] = (GLfloat)0.0; \
	(*matrix)[0] = (*matrix)[5] = (*matrix)[10] = (*matrix)[15] = (GLfloat) 1.0;}

void mat4Set(Mat4 *tomat, Mat4 *frommat);
void mat4SetPos(Mat4 *mat, Xyz *xyz);
void mat4Mult(Mat4 *md, Mat4 *m1, Mat4 *m2);
void mat4MultVec(Xyz *newxyz, Mat4 *mat, Xyz *xyz);
void mat4Perspective(Mat4 *mat, GLfloat fov, GLfloat near, GLfloat far, GLfloat aspratio);
void mat4HeightPerspective(Mat4 *mat, GLfloat height, GLfloat near, GLfloat far, GLfloat aspratio);
void mat4Ortho(Mat4 *mat, GLfloat height, GLfloat near, GLfloat far, GLfloat aspratio);
void mat4Scale(Mat4 *mat, Xyz *s);
void mat4Quat(Mat4 *mat, Quat *q);
void mat4Lookat(Mat4 *mat, Xyz *eye, Xyz *center, Xyz *up);
void mat4InverseLookat(Mat4 *mat, Xyz *eye, Xyz *center, Xyz *up);
void mat4Rotate(Mat4 *mat, Xyz *pos, Xyz *rot, Xyz *scale);
void mat4Inverse(Mat4 *tmat, Mat4 *fmat);
void mat4Place(Mat4 *mat, Xyz *origin, Quat *orient, Xyz *scale);
void mat4Print(Mat4 *mat, const char *matnm);

#endif
