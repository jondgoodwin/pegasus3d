/** Shape type - Handles a 3-D geometry
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new shape */
int shape_new(Value th) {
	pushType(th, getLocal(th, 0), 16); // Create prototype of self (Shape)
	return 1;
}

/** Render the shape */
int shape_render(Value th) {
	// Load the shader into the rendering pipeline */
	pushSym(th, "render");
	pushProperty(th, 0, "shader");
	pushLocal(th, 1);
	getCall(th, 2, 0);

	// Draw the vertexes using the vertex attribute buffers
	GLuint vao;
	GLuint vbo[100];
	int nitems;

	// Get the list of attributes
	Value vertattsym = pushSym(th, "vertexAttributes"); popValue(th);
	Value vertattrlistv = getProperty(th, getLocal(th, 1), vertattsym);
	int nattrs = getSize(vertattrlistv);

	/* Set up Vertex Array Object */
	glGenVertexArrays(1, &vao); /* Allocate */
	glBindVertexArray(vao); /* Bind as currently used */

    /* Allocate and assign as many Vertex Buffer Objects to our handle as we have attributes */
    glGenBuffers(nattrs, vbo);

	for (int i=0; i<nattrs; i++) {
		Value buffer = tblGet(th, getLocal(th, 0), arrGet(th, vertattrlistv, i));

		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]); /* Bind as active */
		glBufferData(GL_ARRAY_BUFFER, getSize(buffer), toStr(buffer), GL_STATIC_DRAW); /* Copy data */
		glVertexAttribPointer(i, getNVals(buffer), GL_FLOAT, GL_FALSE, 0, 0); 
		glEnableVertexAttribArray(i); /* Enable as used */

		nitems = getNStructs(buffer);
	}

	/* Draw */
    glDrawArrays(GL_POLYGON /*prim->mode*/, 0, nitems);

	/* Clean up allocated array and buffers */
	for (int i=0; i<nattrs; i++) {
		glDisableVertexAttribArray(i);
	}
    glDeleteBuffers(nattrs, vbo);
    glDeleteVertexArrays(1, &vao);
	return 1;
}

/* What to do for indexed vertices ...
	unsigned short indices;
	// Load indices

	// Generate a buffer for the indices
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Draw the triangles
	glDrawElements(
		GL_TRIANGLES,      // mode
		indices.size(),    // count
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
	);
 */

/** Initialize shape type */
void shape_init(Value th) {
	Value Shape = pushType(th, aNull, 16);
		pushCMethod(th, shape_new);
		popProperty(th, 0, "new");
		pushCMethod(th, shape_render);
		popProperty(th, 0, "render");
	popGloVar(th, "Shape");
}