/** Pegasus3d Browser - main program
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

char vertexshader[] =
	"// Vertex shader for a self-lit mesh where each vertex has its own color\n"
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


GLfloat diamondpts[4 * 3] = {
     0.0,  1.0, 0.0 , /* Top point */
     1.0,  0.0, 0.0 , /* Right point */
     0.0, -1.0, 0.0 , /* Bottom point */
    -1.0,  0.0, 0.0  }; /* Left point */
GLfloat diamondcols[4 * 4] = {
    1.0,  0.0,  0.0, 1.0, /* Red */
    0.0,  1.0,  0.0, 1.0, /* Green */
    0.0,  0.0,  1.0, 1.0, /* Blue */
    1.0,  1.0,  1.0, 1.0 }; /* White */

/* Populate a buffer with several floating point numbers */
void populateBuffer(Value th, GLfloat *floats, int count) {
	int buffindex = getTop(th) - 1;
	while (count--) {
		pushSym(th, "<<");
		pushLocal(th, buffindex);
		pushValue(th, aFloat(*floats++));
		methodCall(th, 2, 0);
	}
}

/** Add a little test world to the default structures.
    This is just until Acorn's compiler is working. */
void test_init(Value th) {
	int topmark = getTop(th);

	// $.scene.parts {+Mesh}
	pushGloVar(th, "$");
	int sceneidx = getTop(th);
	pushMember(th, getTop(th)-1, "scene");
	Value parts = pushArray(th, aNull, 1);
		pushSym(th, "new");
		pushGloVar(th, "Mesh");
		methodCall(th, 1, 1);
		Aint mesh = getTop(th)-1;
			// Populate new mesh
			pushSym(th, "new");
			pushGloVar(th, "Xyzs");
			pushValue(th, anInt(4));
			methodCall(th, 2, 1);
			populateBuffer(th, diamondpts, 12);
			popMember(th, mesh, "position");

			pushSym(th, "new");
			pushGloVar(th, "Colors");
			pushValue(th, anInt(4));
			methodCall(th, 2, 1);
			populateBuffer(th, diamondcols, 16);
			popMember(th, mesh, "color");

			pushSym(th, "new");
			pushGloVar(th, "Shader");
			methodCall(th, 1, 1);
			Aint shader = getTop(th)-1;
				// Populate shader
				pushString(th, aNull, vertexshader);
				popMember(th, shader, "vertex");
				pushString(th, aNull, fragmentshader);
				popMember(th, shader, "fragment");
				pushSym(th, "new");
				pushGloVar(th, "List");
				pushSym(th, "mvpmatrix");
				methodCall(th, 2, 1);
				popMember(th, shader, "uniform");
				pushSym(th, "new");
				pushGloVar(th, "List");
				pushSym(th, "position");
				pushSym(th, "color");
				methodCall(th, 3, 1);
				popMember(th, shader, "in");
			popMember(th, mesh, "shader");
		arrSet(th, parts, 0, popValue(th));
	popMember(th, sceneidx, "parts");

	setTop(th, topmark);
}