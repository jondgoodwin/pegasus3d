/** Camera matrix calculations
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/


#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new camera, based on the View and Matrix mixins */
int camera_new(Value th) {
	int newcam = getTop(th);
	pushType(th, getLocal(th, 0), 16); // Create prototype of self (Camera)
	addMixin(th, getFromTop(th, 0), getLocal(th, 1));
	if (getTop(th)>2)
		addMixin(th, getFromTop(th, 0), getLocal(th, 2));
	// Create a placeholder for the calculated view/projection matrix
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	methodCall(th, 1, 1);
	popMember(th, newcam, "mvpmatrix");
	return 1;
}

/** Return perspective matrix calculated from fov, minDist and maxDist variables
    and viewheight, viewwidth from passed context */
int persp_getmatrix(Value th) {
	// Get the variables
	GLfloat aspratio = (getTop(th)>1)? ((GLfloat)toAint(pushMember(th, 1, "viewWidth")))/((GLfloat)toAint(pushMember(th, 1, "viewHeight"))) : 1.0f;
	bool perspflag = !isFalse(pushProperty(th, 0, "perspective?"));
	GLfloat fovht = toAfloat(pushProperty(th, 0, perspflag? "fov" : "viewHeight"));
	GLfloat mindist = toAfloat(pushProperty(th, 0, "minDist"));
	GLfloat maxdist = toAfloat(pushProperty(th, 0, "maxDist"));

	// Calculate perspective matrix within closure variable and return it
	Mat4 *mat = (Mat4*) toStr(pushCloVar(th, 2));
	if (perspflag)
		mat4Perspective(mat, fovht, mindist, maxdist, aspratio);
	else
		mat4Ortho(mat, fovht, mindist, maxdist, aspratio);
	return 1;
}

/** Return view matrix calculated from position, lookat and up Xyz values */
int lookat_getmatrix(Value th) {
	// Get the variables
	Xyz *position = (Xyz*) toStr(pushProperty(th, 0, "position"));
	Xyz *lookat = (Xyz*) toStr(pushProperty(th, 0, "lookat"));
	Xyz *up = (Xyz*) toStr(pushProperty(th, 0, "up"));

	// Calculate view matrix within closure variable and return it
	mat4Lookat((Mat4*) toStr(pushCloVar(th, 2)), position, lookat, up);
	return 1;
}

/** Add camera's attributes to passed context (parm 1) */
int camera_render(Value th) {
	if (getTop(th)<2)
		return 1; // Context must be passed

	// Store "mvpmatrix" in context, calculated by multiplying
	// the calculated projection and view matrices
	pushSym(th, "projMatrix");
	pushLocal(th, 0);
	pushLocal(th, 1);
	methodCall(th, 2, 1);
	Mat4 *pmat = (Mat4*) toStr(getFromTop(th, 0));
	pushSym(th, "viewMatrix");
	pushLocal(th, 0);
	pushLocal(th, 1);
	methodCall(th, 2, 1);
	Mat4 *vmat = (Mat4*) toStr(getFromTop(th, 0));
	Mat4 *mat = (Mat4*) toStr(pushMember(th, 0, "mvpmatrix"));
	mat4Mult(mat, pmat, vmat);
	popMember(th, 1, "mvpmatrix");
	return 1;
}

/** Initialize Camera type and create an instance in $.camera */
void camera_init(Value th) {
	Value Camera = pushType(th, aNull, 16);
		pushCMethod(th, camera_new);
		popMember(th, 0, "new");
		pushCMethod(th, camera_render);
		popMember(th, 0, "render");
	popGloVar(th, "Camera");

	// Create a PerspectiveProjection mixin
	Value perspectivemixin = pushMixin(th, aNull, aNull, 5);
		pushValue(th, aFloat(50.0)); // fov
		popMember(th, 0, "fov");
		pushValue(th, aFloat(10.0));  // Orthogonal view height
		popMember(th, 0, "viewHeight");
		pushValue(th, aFloat(0.1f)); // minimum distance
		popMember(th, 0, "minDist");
		pushValue(th, aFloat(1000.0)); // maximum distance
		popMember(th, 0, "maxDist");
		pushValue(th, aTrue);
		popMember(th, 0, "perspective?");
		pushCMethod(th, persp_getmatrix);
		pushValue(th, aNull);
		pushSym(th, "new");
		pushGloVar(th, "Matrix4");
		methodCall(th, 1, 1);
		pushClosure(th, 3);
		popMember(th, 0, "projMatrix");
	popGloVar(th, "PerspectiveProjection");

	// Create a lookat mixin
	Value lookatmixin = pushMixin(th, aNull, aNull, 5);
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(3.0f));
		methodCall(th, 4, 1);
		popMember(th, 0, "position");
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		methodCall(th, 4, 1);
		popMember(th, 0, "lookat");
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(1.0f));
		pushValue(th, aFloat(0.0f));
		methodCall(th, 4, 1);
		popMember(th, 0, "up");
		pushCMethod(th, lookat_getmatrix);
		pushValue(th, aNull);
		pushSym(th, "new");
		pushGloVar(th, "Matrix4");
		methodCall(th, 1, 1);
		pushClosure(th, 3);
		popMember(th, 0, "viewMatrix");
	popGloVar(th, "LookatView");

	// $.camera = +Camera
	pushGloVar(th, "$");
	pushSym(th, "new");
	pushValue(th, Camera);
	pushValue(th, lookatmixin);
	pushValue(th, perspectivemixin);
	methodCall(th, 3, 1);
	popMember(th, getTop(th)-2, "camera");
	popValue(th);
}