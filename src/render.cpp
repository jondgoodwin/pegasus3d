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
	if (getTop(th)<2 || !isCData(oldxyzv = getLocal(th, 1))) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *oldxyz = (Xyz*) toHeader(oldxyzv);

	// Obtain 'mvmatrix' from context
	pushProperty(th, 0, "mvmatrix");
	Mat4 *mvmat = (Mat4*) toHeader(getFromTop(th, 0));
	popValue(th);

	// Create placeholder for new, calculated Xyz
	pushSym(th, "New");
	pushGloVar(th, "Xyz");
	getCall(th, 1, 1);
	Xyz *newxyz = (Xyz*) toHeader(getFromTop(th, 0));

	mat4MultVec(newxyz, mvmat, oldxyz);
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