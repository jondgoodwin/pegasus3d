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
	Value matv = pushCData(th, pushProperty(th, 0, "traits"), Mat4Value, 0, sizeof(Mat4));
	Mat4 *mat = toMat4(matv);
	mat4Identity(mat);
	return 1;
}

/** Set the values of a matrix, copying from another matrix or calculating from origin, orientation and scale */
int matrix4_set(Value th) {
	Mat4 *self = toMat4(getLocal(th,0));
	if (getTop(th)>1 && isMat4(getLocal(th, 1)))
		mat4Set(self, toMat4(getLocal(th,1)));
	else {
		int selfidx = 0;
		// Get the properties needed for calculation
		Value originv = (getTop(th)>1 && isXyz(getLocal(th,1)))? getLocal(th,1) : aNull;
		Value orientv = (getTop(th)>2 && isQuat(getLocal(th,2)))? getLocal(th,2) : aNull;
		Value scalev = (getTop(th)>3 && isXyz(getLocal(th,3)))? getLocal(th,3) : aNull;

		if (orientv!=aNull)
			mat4Quat(self, toQuat(orientv));
		else
			mat4Identity(self);
		if (scalev!=aNull)
			mat4Scale(self, toXyz(scalev));
		if (originv!=aNull)
			mat4SetPos(self, toXyz(originv));
	}

	setTop(th, 1);
	return 1;
}

/** Get cell from matrix at specified row, column */
int matrix4_getcell(Value th) {
	if (getTop(th)<3 || !isInt(getLocal(th, 1)) || !isInt(getLocal(th, 2)))
		return 0;
	int cell = 4*toAint(getLocal(th,2)) + toAint(getLocal(th,1));
	if (cell<0 || cell>15)
		return 0;
	Mat4 *self = toMat4(getLocal(th,0));
	pushValue(th, aFloat(*self[cell]));
	return 1;
}

/** Set cell from matrix at specified row, column */
int matrix4_setcell(Value th) {
	if (getTop(th)<4 || !isFloat(getLocal(th,1)) || !isInt(getLocal(th, 2)) || !isInt(getLocal(th, 3)))
		return 0;
	int cell = 4*toAint(getLocal(th,3)) + toAint(getLocal(th,2));
	if (cell<0 || cell>15)
		return 0;
	Mat4 *self = toMat4(getLocal(th,0));
	*self[cell] = toAfloat(getLocal(th,1));
	setTop(th, 1);
	return 1;
}

/** Invert matrix (assumes bottom row is 0,0,0,1 */
int matrix4_inverse(Value th) {
	Mat4 *self = toMat4(getLocal(th,0));
	Mat4 *source = self;
	if (getTop(th)>1 && isMat4(getLocal(th,1)))
		source = toMat4(getLocal(th,1));
	mat4Inverse(self, source);
	setTop(th, 1);
	return 1;
}

/** Return the Mat4 value obtained from multiplying two matrices.
	The first parm must be a matrix.
	If the second parm is an matrix, self holds the result of multiplying the first parm to the second.
	Otherwise, self holds the multiplication of self by the first parameter. */
int matrix4_mult(Value th) {
	if (getTop(th)<2 || !isMat4(getLocal(th,1)))
		return 0;
	Mat4 *self = toMat4(getLocal(th,0));
	Mat4 *mat1 = toMat4(getLocal(th,1));
	if (getTop(th)>2 && isMat4(getLocal(th,2))) {
		Mat4 *mat2 = toMat4(getLocal(th,2));
		mat4Mult(self, mat1, mat2);
	} else {
		mat4Mult(self, self, mat1);
	}
	setTop(th,1);
	return 1;
}

/** Initialize Matrix4 type and mixin */
void mat4_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushSym(th, "Matrix4");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, matrix4_getcell);
			pushCMethod(th, matrix4_setcell);
			pushClosure(th, 2);
			popProperty(th, 1, "[]");
			pushCMethod(th, matrix4_inverse);
			popProperty(th, 1, "Inverse");
			pushCMethod(th, matrix4_set);
			popProperty(th, 1, "Set");
			pushCMethod(th, matrix4_mult);
			popProperty(th, 1, "Mult");
		popProperty(th, 0, "traits");
		pushCMethod(th, matrix4_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Matrix4");
}