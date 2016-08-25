/** Matrix4 transformation matrix
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new identity Matrix4 value. */
int mat4_new(Value th) {
	Value matv = pushCData(th, pushProperty(th, 0, "_newtype"), PegMat4, 0, sizeof(Mat4));
	Mat4 *mat = (Mat4*) toHeader(matv);
	mat4Identity(mat);
	return 1;
}

/** Initialize Matrix4 type and mixin */
void mat4_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushSym(th, "Matrix4");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
		popProperty(th, 0, "_newtype");
		pushCMethod(th, mat4_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Matrix4");
}