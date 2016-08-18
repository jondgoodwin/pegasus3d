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
	if (getTop(th)<2 || !isStr(oldxyzv = getLocal(th, 1))) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *oldxyz = (Xyz*) toStr(oldxyzv);

	// Obtain 'mvmatrix' from context
	pushProperty(th, 0, "mvmatrix");
	Mat4 *mvmat = (Mat4*) toStr(getFromTop(th, 0));
	popValue(th);

	// Create placeholder for new, calculated Xyz
	pushSym(th, "new");
	pushGloVar(th, "Xyz");
	getCall(th, 1, 1);
	Xyz *newxyz = (Xyz*) toStr(getFromTop(th, 0));

	mat4MultVec(newxyz, mvmat, oldxyz);
	return 1;
}

/** Multiply/adjust mvpmatrix and mvmatrix using calculated lookat view matrix */
int render_lookat(Value th) {
	int contextidx = 0;
	int objidx = 1;
	static Xyz defaultpos = {0.0f, 0.0f, 0.0f};
	static Xyz defaultat = {0.0f, 0.0f, -1.0f};
	static Xyz defaultup = {0.0f, 1.0f, 0.0f};
	Mat4 viewmat;

	// Get the properties needed for calculation
	Value positionv = pushProperty(th, objidx, "location");
	Value lookatv = pushProperty(th, objidx, "lookat");
	Value upv = pushProperty(th, objidx, "up");
	Xyz *position = isStr(positionv)? (Xyz*) toStr(positionv) : &defaultpos;
	Xyz *lookat = isStr(lookatv)? (Xyz*) toStr(lookatv) : &defaultat;
	Xyz *up = isStr(upv)? (Xyz*) toStr(upv) : &defaultup;

	// Calculate view matrix 
	mat4InverseLookat(&viewmat, position, lookat, up);

	// mvpmatrix *= viewmat
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *newmat = (Mat4*) toStr(getFromTop(th, 0));
	Value mvpmatrix = pushProperty(th, contextidx, "mvpmatrix"); popValue(th);
	mat4Mult(newmat, (Mat4*) toStr(mvpmatrix), &viewmat);
	popProperty(th, contextidx, "mvpmatrix");

	// mvmatrix *= viewmat
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	newmat = (Mat4*) toStr(getFromTop(th, 0));
	Value mvmatrix = pushProperty(th, contextidx, "mvmatrix"); popValue(th);
	mat4Mult(newmat, (Mat4*) toStr(mvmatrix), &viewmat);
	popProperty(th, contextidx, "mvmatrix");
	return 0;
}

/** Initialize the RenderContext type */
void render_init(Value th) {
	pushType(th, aNull, 6);
		pushCMethod(th, render_new);
		popProperty(th, 0, "new");
		pushCMethod(th, render_cameraXyz);
		popProperty(th, 0, "CameraXyz");
		pushCMethod(th, render_lookat);
		popProperty(th, 0, "Lookat");
	popGloVar(th, "RenderContext");
}