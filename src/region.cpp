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
	int selfidx = getTop(th);
	pushType(th, getLocal(th, 0), 16);

	// Create storage place for the render-calculated matrices
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	popProperty(th, selfidx, "mmatrix");
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	popProperty(th, selfidx, "tmatrix");

	return 1;
}

/** Generic logic for rendering placed things */
int region_render(Value th) {
	int selfidx = 0;
	int contextidx = 1;
	int parentmatidx = 2;
	if (getTop(th)<3)
		return 0; // Context & parent matrix must be passed

	Value parentMatV = getLocal(th, parentmatidx);

	// Create new context that inherits from old
	int newcontextidx = getTop(th);
	pushSym(th, "New");
	pushLocal(th, contextidx);
	getCall(th, 1, 1);

	// Calculate object's transformation matrix
	pushSym(th, "Set");
	pushProperty(th, selfidx, "tmatrix");
	pushProperty(th, selfidx, "origin");
	pushProperty(th, selfidx, "orientation");
	pushProperty(th, selfidx, "scale");
	getCall(th, 4, 1);
	Mat4 *tmat4 = (Mat4*) toHeader(getFromTop(th, 0));

	// mmatrix = parent_mmatrix * objectmatrix
	Mat4 *newmat = (Mat4*) toHeader(pushProperty(th, selfidx, "mmatrix"));
	if (parentMatV!=aNull) {
		Mat4 *parentmat = (Mat4*) toHeader(parentMatV);
		mat4Mult(newmat, parentmat, tmat4);
	}
	else
		mat4Set(newmat, tmat4);

	// Ask if we should render it? (and augment context)
	pushProperty(th, selfidx, "render?"); // Assume it is method
	pushLocal(th, newcontextidx); // make new context self to render? method
	pushLocal(th, selfidx);
	getCall(th, 2, 1);
	Value torender = popValue(th);
	if (torender!=aFalse) {
		pushSym(th, "_RenderIt");
		pushLocal(th, selfidx);
		pushLocal(th, newcontextidx);
		getCall(th, 2, 0);
	}

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
	Value originv = pushProperty(th, selfidx, "origin");
	if (isCDataType(originv,PegVec3)) {
		Xyz *selfxyz = (Xyz*) toHeader(originv);
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
		pushCMethod(th, region_orient);
		popProperty(th, 0, "OrientTo");
	popGloVar(th, "Region");
}