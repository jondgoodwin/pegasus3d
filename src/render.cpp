/** The RenderContext accumulated and used during the render traversal
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new render context */
int render_new(Value th) {
	pushType(th, getLocal(th, 0), 16);
	return 1;
}

/** Convert an Xyz vector from local to camera (eye) coordinates using mvmatrix */
int render_cameraXyz(Value th) {
	Value oldxyzv;
	if (getTop(th)<3 || !isCDataType(oldxyzv = getLocal(th, 1), PegVec3) )
		return 0;
	Xyz *oldxyz = (Xyz*) toHeader(oldxyzv);

	// Calculate 'mvmatrix' from context
	Mat4 *mmatrix = (Mat4*) toHeader(pushProperty(th, 2, "mmatrix")); popValue(th);
	Mat4 *vmatrix = (Mat4*) toHeader(pushProperty(th, 0, "vmatrix")); popValue(th);
	Mat4 mvmatrix;
	mat4Mult(&mvmatrix, vmatrix, mmatrix);

	// Create placeholder for new, calculated Xyz
	pushSym(th, "New");
	pushGloVar(th, "Xyz");
	getCall(th, 1, 1);
	Xyz *newxyz = (Xyz*) toHeader(getFromTop(th, 0));

	mat4MultVec(newxyz, &mvmatrix, oldxyz);
	return 1;
}

/** Initialize the RenderContext type */
void render_init(Value th) {
	pushType(th, aNull, 6);
		pushSym(th, "RenderContext");
		popProperty(th, 0, "_name");
		pushCMethod(th, render_new);
		popProperty(th, 0, "New");
		pushCMethod(th, render_cameraXyz);
		popProperty(th, 0, "CameraXyz");
	popGloVar(th, "RenderContext");
}