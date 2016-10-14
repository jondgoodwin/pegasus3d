/** Matrix4 transformation matrix
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new identity Matrix4 value. */
int matrix4_new(Value th) {
	Value matv = pushCData(th, pushProperty(th, 0, "_newtype"), PegMat4, 0, sizeof(Mat4));
	Mat4 *mat = (Mat4*) toHeader(matv);
	mat4Identity(mat);
	return 1;
}

/** Set the values of a matrix, copying from another matrix or calculating from origin, orientation and scale */
int matrix4_set(Value th) {
	Mat4 *self = (Mat4*) toHeader(getLocal(th,0));
	if (getTop(th)>1 && isCDataType(getLocal(th, 1), PegMat4))
		mat4Set(self, (Mat4*)toHeader(getLocal(th,1)));
	else {
		int selfidx = 0;
		// Get the properties needed for calculation
		Value originv = (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3))? getLocal(th,1) : aNull;
		Value orientv = (getTop(th)>2 && isCDataType(getLocal(th,2),PegVec4))? getLocal(th,2) : aNull;
		Value scalev = (getTop(th)>3 && isCDataType(getLocal(th,3),PegVec3))? getLocal(th,3) : aNull;

		if (orientv!=aNull)
			mat4Quat(self, (Quat*)toHeader(orientv));
		else
			mat4Identity(self);
		if (scalev!=aNull)
			mat4Scale(self, (Xyz*)toHeader(scalev));
		if (originv!=aNull)
			mat4SetPos(self, (Xyz*)toHeader(originv));
	}

	setTop(th, 1);
	return 1;
}

/** Initialize Matrix4 type and mixin */
void mat4_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushSym(th, "Matrix4");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, matrix4_set);
			popProperty(th, 1, "Set");
		popProperty(th, 0, "_newtype");
		pushCMethod(th, matrix4_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Matrix4");
}