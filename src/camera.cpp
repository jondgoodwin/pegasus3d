/** Camera matrix calculations
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/


#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new camera */
int camera_new(Value th) {
	pushType(th, getLocal(th, 0), 16); // Create camera as subtype of Camera
	return 1;
}

/** Calculate new perspective projection matrix using self/context properties and store in mvpmatrix.
	Put identity matrix in mvmatrix */
int camera_perspective(Value th) {
	int selfidx = 0;
	int contextidx = 1;
	// Retrieve needed properties for calculating perspective matrix
	Value fovv = pushProperty(th, selfidx, "fov");
	Value mindistv = pushProperty(th, selfidx, "near");
	Value maxdistv = pushProperty(th, selfidx, "far");
	GLfloat fovht = isFloat(fovv)? toAfloat(fovv) : 50.0f;
	GLfloat mindist = isFloat(mindistv)? toAfloat(mindistv) : 0.1f;
	GLfloat maxdist = isFloat(maxdistv)? toAfloat(maxdistv) : 1000.0f;

	// Calculate window aspect ratio using context's view size properties 
	GLfloat aspratio = ((GLfloat)toAint(pushProperty(th, contextidx, "viewWidth")))/((GLfloat)toAint(pushProperty(th, contextidx, "viewHeight")));

	// Create calculated perspective matrix and store as mvpmatrix
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *mat = (Mat4*) toStr(getFromTop(th, 0));
	mat4Perspective(mat, fovht, mindist, maxdist, aspratio);
	popProperty(th, contextidx, "mvpmatrix");

	// Create identity matrix and store as mvmatrix
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	popProperty(th, contextidx, "mvmatrix");
	popValue(th);
	return 1;
}

/** Calculate orthogonal projection matrix using context properties and store in mvpmatrix.
	Put identity matrix in mvmatrix */
int camera_orthogonal(Value th) {
	int selfidx = 0;
	int contextidx = 1;

	// Retrieve needed properties for calculating orthogonal matrix
	Value fovv = pushProperty(th, selfidx, "viewHeight");
	Value mindistv = pushProperty(th, selfidx, "near");
	Value maxdistv = pushProperty(th, selfidx, "far");
	GLfloat fovht = isFloat(fovv)? toAfloat(fovv) : 10.0f;
	GLfloat mindist = isFloat(mindistv)? toAfloat(mindistv) : 0.1f;
	GLfloat maxdist = isFloat(maxdistv)? toAfloat(maxdistv) : 1000.0f;

	// Calculate window aspect ratio using context's view size properties 
	GLfloat aspratio = ((GLfloat)toAint(pushProperty(th, contextidx, "viewWidth")))/((GLfloat)toAint(pushProperty(th, contextidx, "viewHeight")));

	// Create calculated orthogonal matrix and store as mvpmatrix
	pushSym(th, "new");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *mat = (Mat4*) toStr(getFromTop(th, 0));
	mat4Ortho(mat, fovht, mindist, maxdist, aspratio);
	popProperty(th, contextidx, "mvpmatrix");

	// Create identity matrix and store as mvmatrix
	pushSym(th, "new");
	pushGloVar(th, "Mattrix4");
	getCall(th, 1, 1);
	popProperty(th, contextidx, "mvmatrix");
	return 1;
}

/** Add camera's attributes to passed context (parm 1) */
int camera_render(Value th) {
	int camidx = 0;
	int contextidx = 1;
	if (getTop(th)<2)
		return 0; // Context must be passed

	// Calculate designated projection matrix into context
	Value projmeth = getProperty(th, getLocal(th, camidx), "projection");
	if (projmeth!=aNull) pushValue(th, projmeth);
	else pushSym(th, "Perspective");
	pushLocal(th, camidx);
	pushLocal(th, contextidx);
	getCall(th, 2, 0);

	// Adjust context matrices with calculated view matrix
	Value viewmeth = getProperty(th, getLocal(th, camidx), "view");
	if (viewmeth!=aNull) pushValue(th, viewmeth);
	else pushSym(th, "Lookat");
	pushLocal(th, contextidx);
	pushLocal(th, camidx);
	getCall(th, 2, 0);

	return 0;
}

/** Initialize Camera type and create an instance in $.camera */
void camera_init(Value th) {
	// Camera is a new Region
	pushSym(th, "new");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	Value Camera = getFromTop(th, 0);
		pushCMethod(th, camera_new);
		popProperty(th, 0, "new");
		pushCMethod(th, camera_render);
		popProperty(th, 0, "_RenderIt");

		pushCMethod(th, camera_perspective);
		popProperty(th, 0, "Perspective");
		pushCMethod(th, camera_orthogonal);
		popProperty(th, 0, "Orthogonal");
		pushSym(th, "Perspective");
		popProperty(th, 0, "projection");
		pushValue(th, aFloat(50.0)); // fov
		popProperty(th, 0, "fov");
		pushValue(th, aFloat(10.0));  // Orthogonal view height
		popProperty(th, 0, "viewHeight");
		pushValue(th, aFloat(0.1f)); // minimum distance
		popProperty(th, 0, "near");
		pushValue(th, aFloat(1000.0)); // maximum distance
		popProperty(th, 0, "far");

		pushSym(th, "Lookat");
		popProperty(th, 0, "view");
		pushSym(th, "new");
		pushGloVar(th, "Xyz");
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(0.0f));
		pushValue(th, aFloat(5.0f));
		getCall(th, 4, 1);
		popProperty(th, 0, "location");
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
	popGloVar(th, "Camera");
}