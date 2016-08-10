/** Define the World type and its global instance '$'
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Retrieve saved shader value */
int render_getshader(Value th) {
	pushLocal(th, 1);
	return 1;
}

/** Switch to a different shader in the rendering pipeline */
int render_setshader(Value th) {
	// Save shader value
	Value shader = pushLocal(th, 1);
	popCloVar(th, 2);
	if (shader!=aNull) {
		// Render the shader, passing the context
		pushSym(th, "_Render");
		pushLocal(th, 1);
		pushLocal(th, 0);
		getCall(th, 2, 0);
	} else
		glUseProgram(0);

	return 0;
}

/** Create a new render context */
int render_new(Value th) {
	Value o = getLocal(th, 0);
	pushType(th, getLocal(th, 0), 16);
	Value n = getFromTop(th, 0);
	return 1;
}

/** Initialize the RenderContext type */
void render_init(Value th) {
	pushType(th, aNull, 6);
		pushCMethod(th, render_new);
		popProperty(th, 0, "new");
		pushCMethod(th, render_getshader);
		pushCMethod(th, render_setshader);
		pushValue(th, aNull);
		pushClosure(th, 3);
		popProperty(th, 0, "shader");
	popGloVar(th, "RenderContext");
}