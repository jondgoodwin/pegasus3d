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

	// Use region's "view" method to calculate regionmatrix
	if (aNull == pushProperty(th, selfidx, "view")) {
		popValue(th);
		pushSym(th, "Rotation");
	}
	pushLocal(th, selfidx);
	getCall(th, 1, 1);
	Mat4 *regionmatrix = (Mat4*) toHeader(getFromTop(th, 0));
	popProperty(th, selfidx, "_matrix");

	// Context: mmatrix *= regionmatrix
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *newmat = (Mat4*) toHeader(getFromTop(th, 0));
	Value mmatrix = pushProperty(th, newcontextidx, "mmatrix"); popValue(th);
	mat4Mult(newmat, (Mat4*) toHeader(mmatrix), regionmatrix);
	popProperty(th, newcontextidx, "mmatrix");

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

/** Calculate Quaternion view matrix */
int region_quat(Value th) {
	int objidx = 0;

	// Get the properties needed for calculation
	Value positionv = pushProperty(th, objidx, "location");
	Value orientv = pushProperty(th, objidx, "orientation");
	Value scalev = pushProperty(th, objidx, "scale");
	Value modelmatrix = pushProperty(th, objidx, "mmatrix");

	if (isCData(modelmatrix)) {
		Mat4 *mat4 = (Mat4*) toHeader(modelmatrix);
		if (orientv!=aNull)
			mat4Quat(mat4, (Quat*)toHeader(orientv));
		else
			mat4Identity(mat4);
		if (scalev!=aNull)
			mat4Scale(mat4, (Xyz*)toHeader(scalev));
		if (positionv!=aNull)
			mat4SetPos(mat4, (Xyz*)toHeader(positionv));
	}
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

/** Set orientation property to a quaternion that looks at passed Xyz location */
int region_orient(Value th) {
	int selfidx = 0;

	// Get destination quat in "orientation" property
	Value orientv = pushProperty(th, selfidx, "orientation");
	if (!isCDataType(orientv, PegVec4)) {
		pushSym(th, "New");
		pushGloVar(th, "Quat");
		getCall(th, 1, 1);
		orientv = getFromTop(th, 0);
		popProperty(th, selfidx, "orientation");
	}
	Quat *quat = (Quat*) toHeader(orientv);

	// Calculate normalized vector pointing from self to target
	if (getTop(th)<2 || !isCDataType(getLocal(th,1), PegVec3))
		return 0;
	Xyz toxyz;
	Xyz* toxyz2 = (Xyz*) toHeader(getLocal(th,1));
	toxyz.x = toxyz2->x; toxyz.y = toxyz2->y; toxyz.z = toxyz2->z; 
	Value locationv = pushProperty(th, selfidx, "location");
	if (isCDataType(locationv,PegVec3)) {
		Xyz *selfxyz = (Xyz*) toHeader(locationv);
		toxyz.x -= selfxyz->x;
		toxyz.y -= selfxyz->y;
		toxyz.z -= selfxyz->z;
	}
	xyzNorm(&toxyz, &toxyz);

	// Calculate quaternion to rotate from -z axis to toxyz
    if ((quat->w = 1 - toxyz.z) < 1.e-6f)
    {
        // For plus z, rotate 180 degrees around arbitrary axis.
        quat->w = quat->x = quat->z = 0.0f;
		quat->y = 1.0f;
    }
    else {
		float inorm2 = 1.0f/sqrt(quat->w*quat->w + toxyz.y*toxyz.y + toxyz.x*toxyz.x);
		quat->x = toxyz.y * inorm2;
		quat->y = -toxyz.x * inorm2;
		quat->z = 0.0f;
		quat->w *= inorm2;
	}
	return 0;
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
		pushCMethod(th, region_quat);
		popProperty(th, 0, "Quat");
		pushCMethod(th, region_lookat);
		popProperty(th, 0, "Lookat");
		pushCMethod(th, region_orient);
		popProperty(th, 0, "OrientTo");
	popGloVar(th, "Region");
}