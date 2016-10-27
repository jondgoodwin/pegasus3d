/** Mixin for objects having origin, orientation and scale
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Calculate and return an object's world "mmatrix" from origin, orientation, scale,
	relative to passed (parent's) world mmatrix */
int placement_calcmatrix(Value th) {
	int selfidx = 0;
	int parentmatidx = 1;
	if (getTop(th)<2)
		return 0; // Parent matrix must be passed

	// Get storage location for object's world matrix (mmatrix),
	// creating it if not found
	Value mmatv = pushProperty(th, selfidx, "mmatrix");
	if (mmatv == aNull) {
		pushSym(th, "New");
		pushGloVar(th, "Matrix4");
		getCall(th, 1, 1);
		mmatv = pushValue(th, getFromTop(th, 0));
		popProperty(th, selfidx, "mmatrix");
	}
	Mat4 *mmat = (Mat4*) toHeader(mmatv);

	// Get origin, orientation and scale, then calculate mmatrix
	Mat4 selfmat;
	Value originv = pushProperty(th, selfidx, "origin"); popValue(th);
	Value orientv = pushProperty(th, selfidx, "orientation"); popValue(th);
	Value scalev = pushProperty(th, selfidx, "scale"); popValue(th);
	mat4Place(&selfmat, isCDataType(originv, PegVec3)? (Xyz*)toHeader(originv) : NULL,
		isCDataType(orientv, PegVec4)? (Quat*)toHeader(orientv) : NULL,
		isCDataType(scalev, PegVec3)? (Xyz*)toHeader(scalev) : NULL);

	// mmatrix = parent_mmatrix * selfmatrix
	Value parentMatV = getLocal(th, parentmatidx);
	if (parentMatV!=aNull) {
		Mat4 *parentmat = (Mat4*) toHeader(parentMatV);
		mat4Mult(mmat, parentmat, &selfmat);
	}
	else
		mat4Set(mmat, &selfmat);
	return 1;
}

/** Set orientation property to a quaternion that looks at passed Xyz location */
int placement_orient(Value th) {
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
void placement_init(Value th) {
	pushType(th, aNull, 6);
		pushSym(th, "Placement");
		popProperty(th, 0, "_name");
		pushCMethod(th, placement_orient);
		popProperty(th, 0, "OrientTo");
		pushCMethod(th, placement_calcmatrix);
		popProperty(th, 0, "CalcMatrix");
	popGloVar(th, "Placement");
}