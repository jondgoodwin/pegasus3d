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

	pushSym(th, "New");
	pushGloVar(th, "Rect");
	getCall(th, 1, 1);
	popProperty(th, camidx, "targetRect");
	return 1;
}

/** Calculate and return new perspective projection matrix using self properties */
int camera_perspective(Value th) {
	int selfidx = 0;

	// Retrieve needed properties for calculating perspective matrix
	Value fovv = pushProperty(th, selfidx, "fov");
	Value mindistv = pushProperty(th, selfidx, "near");
	Value maxdistv = pushProperty(th, selfidx, "far");
	GLfloat fovht = isFloat(fovv)? toAfloat(fovv) : 50.0f;
	GLfloat mindist = isFloat(mindistv)? toAfloat(mindistv) : 0.1f;
	GLfloat maxdist = isFloat(maxdistv)? toAfloat(maxdistv) : 1000.0f;

	GLfloat aspratio = 1.0f;
	Value targetrectv = pushProperty(th, selfidx, "targetRect");
	Rect *targetrect;
	if (isRect(targetrectv)) {
		targetrect = toRect(targetrectv);
		aspratio = ((GLfloat)targetrect->w) / ((GLfloat) targetrect->h);
	}

	// Create calculated perspective matrix and store as mvpmatrix
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *mat = toMat4(getFromTop(th, 0));
	mat4Perspective(mat, fovht, mindist, maxdist, aspratio);
	return 1;
}

/** Calculate and return orthogonal projection matrix using self properties */
int camera_orthogonal(Value th) {
	int selfidx = 0;

	// Retrieve needed properties for calculating orthogonal matrix
	Value fovhtv = pushProperty(th, selfidx, "viewHeight");
	Value mindistv = pushProperty(th, selfidx, "near");
	Value maxdistv = pushProperty(th, selfidx, "far");
	GLfloat fovht = isFloat(fovhtv)? toAfloat(fovhtv) : 10.0f;
	GLfloat mindist = isFloat(mindistv)? toAfloat(mindistv) : 0.1f;
	GLfloat maxdist = isFloat(maxdistv)? toAfloat(maxdistv) : 1000.0f;

	GLfloat aspratio = 1.0f;
	Value targetrectv = pushProperty(th, selfidx, "targetRect");
	Rect *targetrect;
	if (isRect(targetrectv)) {
		targetrect = toRect(targetrectv);
		aspratio = ((GLfloat)targetrect->w) / ((GLfloat) targetrect->h);
	}

	// Create calculated orthogonal matrix and store as mvpmatrix
	pushSym(th, "New");
	pushGloVar(th, "Matrix4");
	getCall(th, 1, 1);
	Mat4 *mat = toMat4(getFromTop(th, 0));
	mat4Ortho(mat, fovht, mindist, maxdist, aspratio);
	return 1;
}

/** Convert an Xyz vector from local to camera (eye) coordinates using mvmatrix */
int camera_cameraXyz(Value th) {
	Value oldxyzv;
	if (getTop(th)<3 || !isXyz(oldxyzv = getLocal(th, 1)) )
		return 0;
	Xyz *oldxyz = toXyz(oldxyzv);

	// Calculate 'mvmatrix' from context
	Mat4 *mmatrix = toMat4(pushProperty(th, 2, "mmatrix")); popValue(th);
	Mat4 *vmatrix = toMat4(pushProperty(th, 0, "vmatrix")); popValue(th);
	Mat4 mvmatrix;
	mat4Mult(&mvmatrix, vmatrix, mmatrix);

	// Create placeholder for new, calculated Xyz
	pushSym(th, "New");
	pushGloVar(th, "Xyz");
	getCall(th, 1, 1);
	Xyz *newxyz = toXyz(getFromTop(th, 0));

	mat4MultVec(newxyz, &mvmatrix, oldxyz);
	return 1;
}

/** Prepare a scene node for rendering */
int camera_renderprep(Value th) {
	int selfidx = 0;
	int nodeidx = 1;
	int parentmatidx = 2;

	// Calculate mmatrix from origin, orientation, scale relative to parent
	pushSym(th, "CalcMatrix");
	pushLocal(th, nodeidx);
	pushLocal(th, parentmatidx);
	getCall(th, 2, 0);

	// Node-specific render preparation
	pushSym(th, "_RenderPrep");
	pushLocal(th, nodeidx);
	pushLocal(th, selfidx);
	getCall(th, 2, 0);

	return 0;
}

/** Add camera's attributes to passed context (parm 1) */
int camera_render(Value th) {
	int selfidx = 0;
	int worldidx = getTop(th);
	pushGloVar(th, "$");

	// Switch OpenGL to work within the target (or $window)
	Value targetv;
	pushSym(th, "MakeCurrent");
	if ((targetv=pushProperty(th, selfidx, "target"))==aNull) {
		popValue(th);
		pushGloVar(th, "$window");
	}
	Value targetrectv = pushProperty(th, selfidx, "targetRect");
	getCall(th, 2, 0);
	Rect *targetrect = toRect(targetrectv);

	// Define an OpenGL viewport within target, if specified
	Value viewportv = pushProperty(th, selfidx, "viewport"); popValue(th);
	if (isRect(viewportv)) {
		Rect *viewport = toRect(viewportv);
		glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
		targetrect->h = viewport->h;
		targetrect->w = viewport->w;
		glScissor(viewport->x, viewport->y, viewport->w, viewport->h);
		glEnable(GL_SCISSOR_TEST);
	}
	else
		glViewport(0, 0, targetrect->w, targetrect->h); // just in case

	// Retrieve background fill color
	static ColorInfo black = {0.0f, 0.0f, 0.0f, 1.0f};
	ColorInfo *background = &black;
	Value backv = pushProperty(th, selfidx, "background"); popValue(th);
	if (isColor(backv))
		background = toColor(backv);

	// OpenGL: Clear target buffers for 3D rendering
	glClearColor(background->red, background->green, background->blue, background->alpha);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Closer objects obscure further objects
	glUseProgram(0); // No shader
	if (viewportv!=aNull)
		glDisable(GL_SCISSOR_TEST);

	// Traverse the scene graph's nodes, preparing for the render
	pushSym(th, "_RenderPrep");
	pushLocal(th, selfidx);
	if (pushProperty(th, selfidx, "scene")==aNull) {
		popValue(th);
		pushProperty(th, worldidx, "scene");
	}
	pushValue(th, aNull); // Default for identity matrix
	getCall(th, 3, 0);

	// Calculate designated projection matrix into context
	Value projmeth = pushProperty(th, selfidx, "projection");
	if (projmeth==aNull) {
		popValue(th);
		pushSym(th, "Perspective");
	}
	pushLocal(th, selfidx);
	getCall(th, 1, 1);
	popProperty(th, selfidx, "pmatrix");

	// Obtain "vmatrix" property storage for camera's view matrix
	Value vmatv = pushProperty(th, selfidx, "vmatrix"); popValue(th);
	if (vmatv==aNull) {
		pushSym(th, "New");
		pushGloVar(th, "Matrix4");
		getCall(th, 1, 1);
		vmatv = getFromTop(th, 0);
		popProperty(th, selfidx, "vmatrix");
	}
	Mat4 *vmat = toMat4(vmatv);

	// Calculate camera's matrix, able to convert from camera coordinates to world coordinates
	Mat4 cammat;
	Value originv = pushProperty(th, selfidx, "origin"); popValue(th);
	Value orientv = pushProperty(th, selfidx, "orientation"); popValue(th);
	Value scalev = pushProperty(th, selfidx, "scale"); popValue(th);
	mat4Place(&cammat, isXyz(originv)? toXyz(originv) : NULL,
		isQuat(orientv)? toQuat(orientv) : NULL,
		isXyz(scalev)? toXyz(scalev) : NULL);

	// If camera follows a 3D object, get its "mmatrix" so that camera coordinates are relative to its eye
	int followidx = getTop(th);
	pushProperty(th, selfidx, "following");
	Value followmatv = pushProperty(th, followidx, "mmatrix"); popValue(th);
	if (followmatv!=aNull) {
		Mat4 eyemat;
		Mat4 *followmat = toMat4(followmatv);
		Value eyev = pushProperty(th, followidx, "eye"); popValue(th);
		if (isXyz(eyev)) {
			mat4Set(&eyemat, followmat);
			followmat = &eyemat;
			Xyz *eye = toXyz(eyev);
			Xyz followeye;
			mat4MultVec(&followeye, followmat, eye);
			(*followmat)[12] = followeye.x;
			(*followmat)[13] = followeye.y;
			(*followmat)[14] = followeye.z;
		}
		mat4Mult(vmat, followmat, &cammat);
	}
	else
		mat4Set(vmat, &cammat);
	popValue(th);

	// Invert camera's matrix so it transforms from world coordinates to camera
	mat4Inverse(vmat, vmat);

	// Render camera's (or world's) scene
	pushSym(th, "_Render");
	if (pushProperty(th, selfidx, "scene")==aNull) {
		popValue(th);
		pushProperty(th, worldidx, "scene");
	}
	pushLocal(th, selfidx);
	pushValue(th, aNull); // Default for identity matrix
	getCall(th, 3, 0);

	// Swap buffers to display the rendered target
	if (targetv!=aNull) {
		pushSym(th, "SwapBuffers");
		pushValue(th, targetv);
		getCall(th, 1, 0);
	}

	popValue(th); // '$'
	return 0;
}

/** Initialize Camera type */
void camera_init(Value th) {
	Value Camera = pushType(th, aNull, 16);
		Value Placement = pushGloVar(th, "Placement"); popValue(th);
		addMixin(th, Camera, Placement);
		pushSym(th, "Camera");
		popProperty(th, 0, "_name");
		pushCMethod(th, camera_new);
		popProperty(th, 0, "New");
		pushCMethod(th, camera_render);
		popProperty(th, 0, "_Render");
		pushCMethod(th, camera_renderprep);
		popProperty(th, 0, "_RenderPrep");

		pushCMethod(th, camera_perspective);
		popProperty(th, 0, "Perspective");
		pushCMethod(th, camera_orthogonal);
		popProperty(th, 0, "Orthogonal");
		pushCMethod(th, camera_cameraXyz);
		popProperty(th, 0, "CameraXyz");
	popGloVar(th, "Camera");
}
