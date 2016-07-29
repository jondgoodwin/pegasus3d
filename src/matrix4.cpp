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
	Value matv = pushNumbers(th, pushProperty(th, 0, "newtype"), 1, 16, sizeof(GLfloat), 0);
	Mat4 *mat = (Mat4*) toStr(matv);
	mat4Identity(mat);
	return 1;
}

/** Initialize Matrix4 type and mixin */
void mat4_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
		popProperty(th, 0, "newtype");
		pushCMethod(th, mat4_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Matrix4");
}