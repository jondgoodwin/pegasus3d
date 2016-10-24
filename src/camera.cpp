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
	int camidx = getTop(th);
	pushType(th, getLocal(th, 0), 16); // Create camera as subtype of Camera

	pushSym(th, "Perspective");
	popProperty(th, camidx, "projection");
	pushValue(th, aFloat(50.0)); // fov
	popProperty(th, camidx, "fov");
	pushValue(th, aFloat(10.0));  // Orthogonal view height
	popProperty(th, camidx, "viewHeight");
	pushValue(th, aFloat(0.1f)); // minimum distance
	popProperty(th, camidx, "near");
	pushValue(th, aFloat(1000.0)); // maximum distance
	popProperty(th, camidx, "far");

	pushSym(th, "New");
	pushGloVar(th, "Xyz");
	pushValue(th, aFloat(0.0f));
	pushValue(th, aFloat(0.0f));
	pushValue(th, aFloat(5.0f));
	getCall(th, 4, 1);
	popProperty(th, camidx, "origin");
	pushSym(th, "New");
	pushGloVar(th, "Quat");
	getCall(th, 1, 1);
	popProperty(th, camidx, "orientation");
	return 1;
}

/** Calculate and return new perspective projection matrix using self/context properties */
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
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *mat = (Mat4*) toHeader(getFromTop(th, 0));
	mat4Perspective(mat, fovht, mindist, maxdist, aspratio);
	return 1;
}

/** Calculate and return orthogonal projection matrix using context properties */
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
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *mat = (Mat4*) toHeader(getFromTop(th, 0));
	mat4Ortho(mat, fovht, mindist, maxdist, aspratio);
	return 1;
}

/** Convert an Xyz vector from local to camera (eye) coordinates using mvmatrix */
int camera_cameraXyz(Value th) {
	Value oldxyzv;
	if (getTop(th)<3 || !isCDataType(oldxyzv = getLocal(th, 1), PegVec3) )
		return 0;
	Xyz *oldxyz = (Xyz*) toHeader(oldxyzv);

	// Calculate 'mvmatrix' from context
	Mat4 *mmatrix = (Mat4*) toHeader(pushProperty(th, 2, "mmatrix")); popValue(th);
	Mat4 *vmatrix = (Mat4*) toHeader(pushProperty(th, 0, "vmatrix")); popValue(th);
	Mat4 mvmatrix;
	mat4Mult(&mvmatrix, vmatrix, mmatrix);

	// Create placeholder for new, calculated Xyz
	pushSym(th, "New");
	pushGloVar(th, "Xyz");
	getCall(th, 1, 1);
	Xyz *newxyz = (Xyz*) toHeader(getFromTop(th, 0));

	mat4MultVec(newxyz, &mvmatrix, oldxyz);
	return 1;
}

/** Add camera's attributes to passed context (parm 1) */
int camera_render(Value th) {
	int selfidx = 0;
	int worldidx = getTop(th);
	pushGloVar(th, "$");

	// Put viewHeight and viewWidth into context
	SDL_Rect window_rect;
	SDL_GetDisplayBounds(0, &window_rect);
	pushValue(th, anInt(window_rect.h));
	popProperty(th, selfidx, "viewHeight");
	pushValue(th, anInt(window_rect.w));
	popProperty(th, selfidx, "viewWidth");

	// Switch OpenGL to work within this window and clear the buffers
	pushSym(th, "MakeCurrent");
	pushGloVar(th, "$window");
	getCall(th, 1, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Closer objects obscure further objects

	// Initialize OpenGL rendering style
	glUseProgram(0); // No shader

	// Calculate designated projection matrix into context
	Value projmeth = pushProperty(th, selfidx, "projection");
	if (projmeth==aNull) {
		popValue(th);
		pushSym(th, "Perspective");
	}
	pushLocal(th, selfidx);
	pushLocal(th, selfidx);
	getCall(th, 2, 1);
	popProperty(th, selfidx, "pmatrix");

	// Calculate camera's view matrix, invert and put in "vmatrix"
	pushSym(th, "Set");
	pushProperty(th, selfidx, "vmatrix");
	if (getFromTop(th, 0)==aNull) {
		popValue(th);
		pushSym(th, "New");
		pushGloVar(th, "Matrix4");
		getCall(th, 1, 1);
		pushValue(th, getFromTop(th, 0));
		popProperty(th, selfidx, "vmatrix");
	}
	pushProperty(th, selfidx, "origin");
	pushProperty(th, selfidx, "orientation");
	pushProperty(th, selfidx, "scale");
	getCall(th, 4, 1);
	Mat4 *vmat = (Mat4*) toHeader(getFromTop(th, 0));
	mat4Inverse(vmat, vmat); // Because we want world->camera

	// $.scene._Render(context)
	pushSym(th, "_Render");
	pushProperty(th, worldidx, "scene");
	pushLocal(th, selfidx);
	pushValue(th, aNull);
	getCall(th, 3, 0);

	popValue(th); // '$'
	return 0;
}

/** Initialize Camera type */
void camera_init(Value th) {
	// Camera is a new Region
	pushSym(th, "New");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	Value Camera = getFromTop(th, 0);
		pushSym(th, "Camera");
		popProperty(th, 0, "_name");
		pushCMethod(th, camera_new);
		popProperty(th, 0, "New");
		pushCMethod(th, camera_render);
		popProperty(th, 0, "_RenderIt");

		pushCMethod(th, camera_perspective);
		popProperty(th, 0, "Perspective");
		pushCMethod(th, camera_orthogonal);
		popProperty(th, 0, "Orthogonal");
		pushCMethod(th, camera_cameraXyz);
		popProperty(th, 0, "CameraXyz");
	popGloVar(th, "Camera");
}
