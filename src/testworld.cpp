/** Pegasus3d Browser - main program
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

char vertexshader[] =
	"// Vertex shader for a self-lit shape where each vertex has its own color\n"
	"#version 130\n"
	"uniform mat4 mvpmatrix;"	
	"in  vec3 position;"
	"in  vec4 color;"
	"out vec4 vert_color;"	
	"void main(void) {"
		"gl_Position = mvpmatrix * vec4(position, 1.0);"
		"vert_color = color;"
	"}";

char fragmentshader[] =
	"// Fragment shader passes along interpolated, full opacity vec3 color from vertex shader\n"
	"#version 130\n"
	"in  vec4 vert_color;\n"
	"void main(void) {\n"
		"gl_FragColor = vert_color;\n"
	"}\n";

char diamondptstr[] = 
     "0.0,  1.0, 0.0 ,"
     "1.0,  0.0, 0.0 ,"
     "0.0, -1.0, 0.0 ,"
    "-1.0,  0.0, 0.0  ";

char diamondcolstr[] =
    "1.0,  0.0,  0.0, 1.0,"
    "0.0,  1.0,  0.0, 1.0,"
    "0.0,  0.0,  1.0, 1.0,"
    "1.0,  1.0,  1.0, 1.0 ";

/* Populate a buffer with several floating point numbers */
void populateBuffer(Value th, GLfloat *floats, int count) {
	int buffindex = getTop(th) - 1;
	while (count--) {
		pushSym(th, "<<");
		pushLocal(th, buffindex);
		pushValue(th, aFloat(*floats++));
		getCall(th, 2, 0);
	}
}

/** Add a little test world to the default structures.
    This is just until Acorn's compiler is working. */
void test_init(Value th) {
	int topmark = getTop(th);

	// $.scene.parts {+Shape}
	pushGloVar(th, "$");
	int sceneidx = getTop(th);
	pushProperty(th, getTop(th)-1, "scene");
	Value parts = pushArray(th, aNull, 1);
		pushSym(th, "new");
		pushGloVar(th, "Shape");
		getCall(th, 1, 1);
		Aint shape = getTop(th)-1;
			// Populate new shape
			pushSym(th, "Polygon");
			popSetActProp(th, shape, "draw");

			pushSym(th, "new");
			pushGloVar(th, "Xyzs");
			pushString(th, aNull, diamondptstr);
			getCall(th, 2, 1);
			popProperty(th, shape, "position");

			pushSym(th, "new");
			pushGloVar(th, "Colors");
			pushString(th, aNull, diamondcolstr);
			getCall(th, 2, 1);
			popProperty(th, shape, "color");

			pushSym(th, "new");
			pushGloVar(th, "Shader");
			getCall(th, 1, 1);
			Aint shader = getTop(th)-1;
				// Populate shader
				pushString(th, aNull, vertexshader);
				popProperty(th, shader, "vertex");
				pushString(th, aNull, fragmentshader);
				popProperty(th, shader, "fragment");
				pushSym(th, "new");
				pushGloVar(th, "List");
				pushSym(th, "mvpmatrix");
				getCall(th, 2, 1);
				popProperty(th, shader, "uniforms");
				pushSym(th, "new");
				pushGloVar(th, "List");
				pushSym(th, "position");
				pushSym(th, "color");
				getCall(th, 3, 1);
				popProperty(th, shader, "attributes");
			popProperty(th, shape, "shader");
		arrSet(th, parts, 0, popValue(th));
	popProperty(th, sceneidx, "parts");

	setTop(th, topmark);
}