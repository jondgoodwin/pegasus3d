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
	// Ensure parameters have at least the default mixins
	if (getTop(th)<2)
		pushGloVar(th, "LookatView");
	if (getTop(th)<3)
		pushGloVar(th, "PerspectiveProjection");

	int newcamidx = getTop(th);
	Value newcam = pushType(th, getLocal(th, 0), 16); // Create camera as subtype of Camera

	// Add view and perspective mixins from parameters
	addMixin(th, newcam, getLocal(th, 1));
	addMixin(th, newcam, getLocal(th, 2));

	// Create mvpmatrix property to hold the calculated view/projection matrix
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	popProperty(th, newcamidx, "mvpmatrix");
	return 1;
}

/** Return perspective matrix calculated from fov, near and far variables
    and viewheight, viewwidth from passed context */
int persp_getmatrix(Value th) {
	// Get the variables
	GLfloat aspratio = (getTop(th)>1)? ((GLfloat)toAint(pushProperty(th, 1, "viewWidth")))/((GLfloat)toAint(pushProperty(th, 1, "viewHeight"))) : 1.0f;
	bool perspflag = !isFalse(pushProperty(th, 0, "perspective?"));
	GLfloat fovht = toAfloat(pushProperty(th, 0, perspflag? "fov" : "viewHeight"));
	GLfloat mindist = toAfloat(pushProperty(th, 0, "near"));
	GLfloat maxdist = toAfloat(pushProperty(th, 0, "far"));

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
	getCall(th, 2, 1);
	Mat4 *pmat = (Mat4*) toStr(getFromTop(th, 0));
	pushSym(th, "viewMatrix");
	pushLocal(th, 0);
	pushLocal(th, 1);
	getCall(th, 2, 1);
	Mat4 *vmat = (Mat4*) toStr(getFromTop(th, 0));
	Mat4 *mat = (Mat4*) toStr(pushProperty(th, 0, "mvpmatrix"));
	mat4Mult(mat, pmat, vmat);
	popProperty(th, 1, "mvpmatrix");
	return 1;
}

/** Initialize Camera type and create an instance in $.camera */
void camera_init(Value th) {
	pushType(th, aNull, 16);
		pushCMethod(th, camera_new);
		popProperty(th, 0, "new");
		pushCMethod(th, camera_render);
		popProperty(th, 0, "render");
	popGloVar(th, "Camera");

	// Create a PerspectiveProjection mixin
	pushMixin(th, aNull, aNull, 5);
		pushValue(th, aFloat(50.0)); // fov
		popProperty(th, 0, "fov");
		pushValue(th, aFloat(10.0));  // Orthogonal view height
		popProperty(th, 0, "viewHeight");
		pushValue(th, aFloat(0.1f)); // minimum distance
		popProperty(th, 0, "near");
		pushValue(th, aFloat(1000.0)); // maximum distance
		popProperty(th, 0, "far");
		pushValue(th, aTrue);
		popProperty(th, 0, "perspective?");
		pushCMethod(th, persp_getmatrix);
		pushValue(th, aNull);
		pushSym(th, "new");
		pushGloVar(th, "Matrix4");
		getCall(th, 1, 1);
		pushClosure(th, 3);
		popProperty(th, 0, "projMatrix");
	popGloVar(th, "PerspectiveProjection");

	// Create a lookat mixin
	pushMixin(th, aNull, aNull, 5);
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(3.0f));
		getCall(th, 4, 1);
		popProperty(th, 0, "position");
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		getCall(th, 4, 1);
		popProperty(th, 0, "lookat");
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(1.0f));
		pushValue(th, aFloat(0.0f));
		getCall(th, 4, 1);
		popProperty(th, 0, "up");
		pushCMethod(th, lookat_getmatrix);
		pushValue(th, aNull);
		pushSym(th, "new");
		pushGloVar(th, "Matrix4");
		getCall(th, 1, 1);
		pushClosure(th, 3);
		popProperty(th, 0, "viewMatrix");
	popGloVar(th, "LookatView");
}