/** Define the World type and its global instance '$'
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Generic logic for rendering placed things */
int shared_render(Value th) {
	int selfidx = 0;
	int contextidx = 1;
	if (getTop(th)<2)
		return 0; // Context must be passed

	// Create new context that inherits from old
	int newcontextidx = getTop(th);
	pushSym(th, "new");
	pushLocal(th, contextidx);
	getCall(th, 1, 1);

	// Adjust context matrices with calculated view matrix
	Value viewmeth = getProperty(th, getLocal(th, selfidx), "view");
	if (viewmeth!=aNull) pushValue(th, viewmeth);
	else pushSym(th, "Rotate");
	pushLocal(th, newcontextidx);
	pushLocal(th, selfidx);
	getCall(th, 2, 0);

	// Ask if we should render it? (and augment context)
	pushProperty(th, selfidx, "render?"); // Assume it is method
	pushLocal(th, newcontextidx); // make new context self to render? method
	getCall(th, 1, 1);
	Value torender = popValue(th);
	if (torender!=aFalse) {
		pushSym(th, "_RenderIt");
		pushLocal(th, selfidx);
		pushLocal(th, newcontextidx);
		getCall(th, 2, 0);
	}

	return 1;
}

/** Create a new render context */
int render_new(Value th) {
	Value o = getLocal(th, 0);
	pushType(th, getLocal(th, 0), 16);
	Value n = getFromTop(th, 0);
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
	mat4Lookat(&viewmat, position, lookat, up);

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

/** Multiply/adjust mvpmatrix and mvmatrix using calculated rotation view matrix */
int render_rotationview(Value th) {
	int contextidx = 0;
	int objidx = 1;
	static Xyz defaultpos = {0.0f, 0.0f, 0.0f};
	static Xyz defaultrot = {0.0f, 0.0f, 0.0f};
	static Xyz defaultscale = {1.0f, 1.0f, 1.0f};
	Mat4 viewmat;

	// Get the properties needed for calculation
	Value positionv = pushProperty(th, objidx, "location");
	Value rotv = pushProperty(th, objidx, "rotation");
	Value scalev = pushProperty(th, objidx, "scale");
	Xyz *position = isStr(positionv)? (Xyz*) toStr(positionv) : &defaultpos;
	Xyz *rotate = isStr(rotv)? (Xyz*) toStr(rotv) : &defaultrot;
	Xyz *scale = isStr(scalev)? (Xyz*) toStr(scalev) : &defaultscale;

	// Calculate view matrix 
	mat4Rotate(&viewmat, position, rotate, scale);

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
		pushCMethod(th, render_lookat);
		popProperty(th, 0, "Lookat");
		pushCMethod(th, render_rotationview);
		popProperty(th, 0, "Rotate");
	popGloVar(th, "RenderContext");
}