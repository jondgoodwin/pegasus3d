/** Abstract type used by types that occupy a 3D region in world space.
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new region */
int region_new(Value th) {
	int region = getTop(th);
	pushType(th, getLocal(th, 0), 16);

	// Create storage place for the render-calculated view matrix
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	popProperty(th, region, "mmatrix");

	return 1;
}

/** Generic logic for rendering placed things */
int region_render(Value th) {
	int selfidx = 0;
	int contextidx = 1;
	if (getTop(th)<2)
		return 0; // Context must be passed

	// Create new context that inherits from old
	int newcontextidx = getTop(th);
	pushSym(th, "New");
	pushLocal(th, contextidx);
	getCall(th, 1, 1);

	// Use "view"'s method to calculate 'mmodel'
	Value viewmeth = pushProperty(th, selfidx, "view"); popValue(th);
	if (viewmeth!=aNull) pushValue(th, viewmeth);
	else pushSym(th, "Rotation");
	pushLocal(th, selfidx);
	getCall(th, 1, 1);
	Mat4 *viewmat = (Mat4*) toHeader(getFromTop(th, 0));

	// Context: mvmatrix *= viewmat
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *newmat = (Mat4*) toHeader(getFromTop(th, 0));
	Value mvmatrix = pushProperty(th, newcontextidx, "mvmatrix"); popValue(th);
	mat4Mult(newmat, (Mat4*) toHeader(mvmatrix), viewmat);
	popProperty(th, newcontextidx, "mvmatrix");

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

/** Calculate rotation view matrix */
int region_rotation(Value th) {
	int objidx = 0;
	static Xyz defaultpos = {0.0f, 0.0f, 0.0f};
	static Xyz defaultrot = {0.0f, 0.0f, 0.0f};
	static Xyz defaultscale = {1.0f, 1.0f, 1.0f};

	// Get the properties needed for calculation
	Value positionv = pushProperty(th, objidx, "location");
	Value rotv = pushProperty(th, objidx, "rotation");
	Value scalev = pushProperty(th, objidx, "scale");
	Xyz *position = isCData(positionv)? (Xyz*) toHeader(positionv) : &defaultpos;
	Xyz *rotate = isCData(rotv)? (Xyz*) toHeader(rotv) : &defaultrot;
	Xyz *scale = isCData(scalev)? (Xyz*) toHeader(scalev) : &defaultscale;

	// Calculate view matrix, storing it in 'mmatrix'
	Value modelmatrix = pushProperty(th, objidx, "mmatrix");
	if (isCData(modelmatrix))
		mat4Rotate((Mat4*)toHeader(modelmatrix), position, rotate, scale);
	
	return 1;
}

/** Calculate lookat view matrix */
int region_lookat(Value th) {
	int objidx = 0;
	static Xyz defaultpos = {0.0f, 0.0f, 0.0f};
	static Xyz defaultat = {0.0f, 0.0f, -1.0f};
	static Xyz defaultup = {0.0f, 1.0f, 0.0f};

	// Get the properties needed for calculation
	Value positionv = pushProperty(th, objidx, "location");
	Value lookatv = pushProperty(th, objidx, "lookat");
	Value upv = pushProperty(th, objidx, "up");
	Xyz *position = isCData(positionv)? (Xyz*) toHeader(positionv) : &defaultpos;
	Xyz *lookat = isCData(lookatv)? (Xyz*) toHeader(lookatv) : &defaultat;
	Xyz *up = isCData(upv)? (Xyz*) toHeader(upv) : &defaultup;

	// Calculate view matrix, storing it in 'mmatrix'
	Value modelmatrix = pushProperty(th, objidx, "mmatrix");
	if (isCData(modelmatrix))
		mat4Lookat((Mat4*)toHeader(modelmatrix), position, lookat, up);

	return 1;
}

/** Initialize the RenderContext type */
void region_init(Value th) {
	pushType(th, aNull, 6);
		pushSym(th, "Region");
		popProperty(th, 0, "_name");
		pushCMethod(th, region_new);
		popProperty(th, 0, "New");
		pushCMethod(th, region_render);
		popProperty(th, 0, "_Render");
		pushCMethod(th, region_rotation);
		popProperty(th, 0, "Rotation");
		pushCMethod(th, region_lookat);
		popProperty(th, 0, "Lookat");
	popGloVar(th, "Region");
}