/** Shape type - Handles a 3-D geometric mesh
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/
#include "pegasus3d.h"

/** Get the draw property's closure value: OpenGL integer value */
int shape_getDraw(Value th) {
	pushCloVar(th, 2);
	return 1;
}

/** Set the draw property's closure value using symbols */
int shape_setDraw(Value th) {
	Value primtbl = pushProperty(th, 0, "DrawPrimitives");
	Value drawprim = pushValue(th, tblGet(th, primtbl, getLocal(th, 1)));
	if (drawprim == aNull) {
		pushValue(th, anInt(GL_TRIANGLES));
	}
	popCloVar(th, 2);
	return 1;
}

/** Create a new shape */
int shape_new(Value th) {
	AuintIdx newShapeIdx = getTop(th);
	pushType(th, getLocal(th, 0), 16); // subtype from Shape

	// Create a new closure for 'draw', to lookup primitive only on set
	pushCMethod(th, shape_getDraw);
	pushCMethod(th, shape_setDraw);
	pushValue(th, anInt(GL_TRIANGLES));
	pushClosure(th, 3);
	popProperty(th, newShapeIdx, "draw");

	return 1;
}

/** Render the shape */
int shape_render(Value th) {
	int selfidx = 0;
	int contextidx = 1;

	// Create new context that inherits from old
	int newcontextidx = getTop(th);
	pushSym(th, "new");
	pushLocal(th, contextidx);
	getCall(th, 1, 1);

	// Ask if we should render it? (and augment context)
	pushProperty(th, selfidx, "render?"); // Assume it is method
	pushLocal(th, newcontextidx); // make new context self to render? method
	getCall(th, 1, 1);
	Value torender = popValue(th);
	if (torender!=aFalse) {

		// Draw the vertexes using the vertex attribute buffers
		GLuint vao;
		GLuint vbo[100];
		int nverts = -1;

		// Get the list of attributes
		Value vertattsym = pushSym(th, "vertexAttributes"); popValue(th);
		Value vertattrlistv = getProperty(th, getLocal(th, newcontextidx), vertattsym);
		int nattrs = getSize(vertattrlistv);

		/* Set up Vertex Array Object */
		glGenVertexArrays(1, &vao); /* Allocate */
		glBindVertexArray(vao); /* Bind as currently used */

		/* Allocate and assign as many Vertex Buffer Objects to our handle as we have attributes */
		glGenBuffers(nattrs, vbo);

		for (int i=0; i<nattrs; i++) {
			Value buffer = tblGet(th, getLocal(th, selfidx), arrGet(th, vertattrlistv, i));

			// Bind as active, copy, define and enable the OpenGL buffer
			glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
			glBufferData(GL_ARRAY_BUFFER, getSize(buffer), toStr(buffer), GL_STATIC_DRAW); /* Copy data */
			if (nbrIsInteger(buffer)) {
				switch (nbrGetValSz(buffer)) {
				case 1: glVertexAttribPointer(i, nbrGetNVals(buffer), GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
				case 2: glVertexAttribPointer(i, nbrGetNVals(buffer), GL_UNSIGNED_SHORT, GL_FALSE, 0, 0);
				case 4: glVertexAttribPointer(i, nbrGetNVals(buffer), GL_INT, GL_FALSE, 0, 0);
				}
			}
			else
				glVertexAttribPointer(i, nbrGetNVals(buffer), GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(i);

			// Remember the smallest number of vertices we found in the buffers
			nverts = (nverts < 0 || nverts>nbrGetNStructs(buffer))? nbrGetNStructs(buffer) : nverts;
		}

		/* Draw specified primitives using vertices defined by buffers */
		glDrawArrays(toAint(pushGetActProp(th, selfidx, "draw")), 0, nverts);
		popValue(th);

		/* Clean up alloc	ated array and buffers */
		for (int i=0; i<nattrs; i++) {
			glDisableVertexAttribArray(i);
		}
		glDeleteBuffers(nattrs, vbo);
		glDeleteVertexArrays(1, &vao);
	}
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
	Value Shape = pushType(th, aNull, 8);
		pushCMethod(th, shape_new);
		popProperty(th, 0, "new");
		pushCMethod(th, shape_render);
		popProperty(th, 0, "_Render");
		pushTbl(th, aNull, 15);
			pushValue(th, anInt(GL_POINTS));
			popTblSet(th, 1, "Points");
			pushValue(th, anInt(GL_LINE_STRIP));
			popTblSet(th, 1, "LineStrip");
			pushValue(th, anInt(GL_LINE_LOOP));
			popTblSet(th, 1, "LineLoop");
			pushValue(th, anInt(GL_LINES));
			popTblSet(th, 1, "Lines");
			pushValue(th, anInt(GL_LINE_STRIP_ADJACENCY));
			popTblSet(th, 1, "LineStripAdjacency");
			pushValue(th, anInt(GL_LINES_ADJACENCY));
			popTblSet(th, 1, "LinesAdjacency");
			pushValue(th, anInt(GL_TRIANGLE_STRIP));
			popTblSet(th, 1, "TriangleStrip");
			pushValue(th, anInt(GL_TRIANGLE_FAN));
			popTblSet(th, 1, "TriangleFan");
			pushValue(th, anInt(GL_TRIANGLES));
			popTblSet(th, 1, "Triangles");
			pushValue(th, anInt(GL_TRIANGLE_STRIP_ADJACENCY));
			popTblSet(th, 1, "TriangleStripAdjacency");
			pushValue(th, anInt(GL_TRIANGLES_ADJACENCY));
			popTblSet(th, 1, "TrianglesAdjacency");
			pushValue(th, anInt(GL_PATCHES));
			popTblSet(th, 1, "Patches");
			pushValue(th, anInt(GL_POLYGON));
			popTblSet(th, 1, "Polygon");
		popProperty(th, 0, "DrawPrimitives");
	popGloVar(th, "Shape");
}