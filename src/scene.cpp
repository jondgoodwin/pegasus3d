/** Scene definition and rendering
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new scene */
int scene_new(Value th) {
	pushType(th, getLocal(th, 0), 16); // Create prototype of self (Scene)
	return 1;
}

/** Get background color */
int scene_getbackground(Value th) {
	pushCloVar(th, 2);
	return 1;
}

/** Set background color with parm 1 as Color */
int scene_setbackground(Value th) {
	if (getTop(th)>1) {
		pushLocal(th, 1);
		popCloVar(th, 2);
		pushLocal(th, 1);
	}
	return 1;
}

/** Render scene using passed context (parm 1) */
int scene_render(Value th) {
	if (getTop(th)<2)
		return 1; // Context must be passed

	// Clear the drawing and depth buffers using background color. Enable Z depth testing
	float r, g, b, a;
	pushSym(th, "unpack");
	pushSym(th, "background");
	pushLocal(th, 0);
	getCall(th, 1, 1);
	getCall(th, 1, 4);
	a = toAfloat(popValue(th));
	b = toAfloat(popValue(th));
	g = toAfloat(popValue(th));
	r = toAfloat(popValue(th));
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Closer objects obscure further objects

	// Render each of the scene's parts, in order
	Value parts = pushProperty(th, 0, "parts");
	if (isArr(parts)) {
		Aint sz = getSize(parts);
		for (Aint i=0; i<sz; i++) {
			pushSym(th, "render");
			pushValue(th, arrGet(th, parts, i));
			pushLocal(th, 1);
			getCall(th, 2, 0);
		}
	}

	/* Clean up */
	glUseProgram(0);

	return 1;
}

/** Initialize Scene type and create an instance in $.scene */
void scene_init(Value th) {
	pushType(th, aNull, 16);
		pushCMethod(th, scene_new);
		popProperty(th, 0, "new");
		pushCMethod(th, scene_render);
		popProperty(th, 0, "render");
		// background: +Closure(., ., +Color(0,0,0))
		pushCMethod(th, scene_getbackground);
		pushCMethod(th, scene_setbackground);
		pushSym(th, "new");
		pushGloVar(th, "Color");
		pushValue(th, aFloat(0.0));
		pushValue(th, aFloat(0.0));
		pushValue(th, aFloat(0.0));
		getCall(th, 4, 1);
		pushClosure(th, 3);
		popProperty(th, 0, "background");
	popGloVar(th, "Scene");
}
