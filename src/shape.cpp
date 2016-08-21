/** Shape type - Handles a 3-D geometric mesh
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/
#include "pegasus3d.h"
#include "xyzmath.h"
#include <math.h>

/** Generate a sphere shape, passing radius and nsegments */
int shape_sphere(Value th) {
	// Obtain and validate parameters
	float radius = getTop(th)>=2 && isFloat(getLocal(th, 1))? toAfloat(getLocal(th,1)) : 1.0f;
	int nsegments = getTop(th)>=3 && isInt(getLocal(th, 2))? toAint(getLocal(th,2)) : 15;
	int nverts = nsegments * (nsegments-2) + 2;
	int nindices = 6*nsegments*nsegments;
	if (nsegments<=2 || nverts>=65536) {
		pushValue(th, aNull);
		return 1;
	}

	// Push a new Shape on the stack and give it buffer properties
	int sphereidx = getTop(th);
	pushSym(th, "new");
	pushGloVar(th, "Shape");
	getCall(th, 1, 1);
	pushSym(th, "new");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value posval = getFromTop(th, 0);
	popProperty(th, sphereidx, "positions");
	pushSym(th, "new");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value normval = getFromTop(th, 0);
	popProperty(th, sphereidx, "normals");
	pushSym(th, "new");
	pushGloVar(th, "Integers");
	pushValue(th, anInt(nindices));
	getCall(th, 2, 1);
	Value indxval = getFromTop(th, 0);
	popProperty(th, sphereidx, "indices");

    // Top pole vertex
	GLfloat zero = 0.f;
	GLfloat one = 1.f;
	strAppend(th, posval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, posval, (const char*)(&radius), sizeof(GLfloat));
	strAppend(th, posval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, normval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, normval, (const char*)(&one), sizeof(GLfloat));
	strAppend(th, normval, (const char*)(&zero), sizeof(GLfloat));

    // each vertex,  vertically and then around the circle
	int v0, v1, v2, v3;
	for (int i=1; i<nsegments; i++) {
		GLfloat mang = (GLfloat)M_PI * ((GLfloat)i) / ((GLfloat)(nsegments));
		for (int j=0; j<nsegments; j++) {
			GLfloat nang = 2.0f * (GLfloat)M_PI * ((GLfloat)j) / ((GLfloat)nsegments); 
			GLfloat x = -cos(nang)*sin(mang);
			GLfloat y = cos(mang);
			GLfloat z = sin(nang)*sin(mang);

			strAppend(th, normval, (const char*)(&x), sizeof(GLfloat));
			strAppend(th, normval, (const char*)(&y), sizeof(GLfloat));
			strAppend(th, normval, (const char*)(&z), sizeof(GLfloat));
			x *= radius;
			y *= radius;
			z *= radius;
			strAppend(th, posval, (const char*)(&x), sizeof(GLfloat));
			strAppend(th, posval, (const char*)(&y), sizeof(GLfloat));
			strAppend(th, posval, (const char*)(&z), sizeof(GLfloat));

			// Generate two triangles per segment
			if (i==1) {
				v0 = 0;
				v1 = (j==nsegments-1)? 1 : j+2;
				v2 = j+1;
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v1), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
			} else {
				v0 = ((i-2)*nsegments)+j+1;
				v1 = ((i-2)*nsegments)+((j==nsegments-1)? 1 : j+2);
				v2 = ((i-1)*nsegments)+((j==nsegments-1)? 1 : j+2);
				v3 = ((i-1)*nsegments)+j+1;
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v3), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v1), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
			}
		}
	}

	// Bottom pole vertex
	radius = -radius;
	one = -one;
	strAppend(th, posval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, posval, (const char*)(&radius), sizeof(GLfloat));
	strAppend(th, posval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, normval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, normval, (const char*)(&one), sizeof(GLfloat));
	strAppend(th, normval, (const char*)(&zero), sizeof(GLfloat));

	for (int j=0; j<nsegments; j++) {
		v0 = ((nsegments-2)*nsegments)+j+1;
		v1 = (j==nsegments-1)? ((nsegments-2)*nsegments)+1 : ((nsegments-2)*nsegments)+j+2;
		v2 = ((nsegments-1)*nsegments)+1;
		strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
		strAppend(th, indxval, (const char*)(&v1), sizeof(unsigned short));
		strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
	}

	return 1;
}

/** Get the draw property's closure value: OpenGL integer value */
int shape_getDraw(Value th) {
	pushCloVar(th, 2);
	return 1;
}

/** Set the draw property's closure value using symbols */
int shape_setDraw(Value th) {
	Value primtbl = pushProperty(th, 0, "_$drawPrimitives");
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
int shape_renderit(Value th) {
	int selfidx = 0;
	int contextidx = 1;

	// Render the shader, loading it and its uniforms
	pushSym(th, "_Render");
	Value shader = pushProperty(th, contextidx, "shader");
	pushLocal(th, contextidx);
	getCall(th, 2, 0);

	// Draw the vertexes using the vertex attribute buffers
	GLuint vao;
	GLuint vbo[100];
	unsigned int nverts = -1;

	// Get the list of vertex attributes
	Value vertattsym = pushSym(th, "attributes");
	Value vertattrlistv = getProperty(th, shader, vertattsym);
	popValue(th); // symbol
	int nattrs = getSize(vertattrlistv);

	// Figure out who to get attributes from
	Value attrsource = pushProperty(th, selfidx, "shape");
	if (attrsource == aNull)
		attrsource = getLocal(th, selfidx);
	popValue(th);

	// Set up Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Allocate and assign as many Vertex Buffer Objects to our handle as we have attributes
	// then load and activate each one. 
	glGenBuffers(nattrs, vbo);
	for (int i=0; i<nattrs; i++) {
		Value buffer = getProperty(th, attrsource, arrGet(th, vertattrlistv, i));
		if (!isCData(buffer))
			continue;
		ArrayHeader *buffhdr = (ArrayHeader*) toHeader(buffer);

		// Bind as active, copy, define and enable the OpenGL buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
		glBufferData(GL_ARRAY_BUFFER, getSize(buffer), toCData(buffer), GL_STATIC_DRAW); /* Copy data */
		AuintIdx n = getSize(buffer);
		float *f = (float *)toCData(buffer);
		switch (buffhdr->mbrType) {
		case PegUint8: glVertexAttribPointer(i, buffhdr->structSz, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0); break;
		case PegUint16: glVertexAttribPointer(i, buffhdr->structSz, GL_UNSIGNED_SHORT, GL_FALSE, 0, 0); break;
		case PegUint32: glVertexAttribPointer(i, buffhdr->structSz, GL_INT, GL_FALSE, 0, 0); break;
		case PegFloat: case PegVec2: case PegVec3: case PegVec4:
			glVertexAttribPointer(i, buffhdr->structSz, GL_FLOAT, GL_FALSE, 0, 0); break;
		default: ;
		}
		glEnableVertexAttribArray(i);

		// Remember the smallest number of vertices we found in the buffers
		nverts = (nverts < 0 || nverts>buffhdr->nStructs)? buffhdr->nStructs : nverts;
	}

	// How shall we draw them?
	Value drawprop = pushGetActProp(th, selfidx, "draw");
	int drawmode = isInt(drawprop)? toAint(drawprop) : GL_TRIANGLES;
	popValue(th);

	/* Do we have a "indices" property with vertex indices? Use it */
	Value indicesym = pushSym(th, "indices");
	Value vertices = getProperty(th, attrsource, indicesym);
	popValue(th);
	if (isCData(vertices)) {
		ArrayHeader *verthdr = (ArrayHeader*)toHeader(vertices);
		// Generate a buffer for the indices
		GLuint elementbuffer;
		glGenBuffers(1, &elementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, getSize(vertices), toCData(vertices), GL_STATIC_DRAW);

		// Draw the vertices using the indices as a guide
		glDrawElements(drawmode, verthdr->nStructs, GL_UNSIGNED_SHORT, (void*)0);
	}
	/* Otherwise, draw specified primitives using vertices defined by attribute buffers */
	else
		glDrawArrays(drawmode, 0, nverts);
	popValue(th); // vertices

	/* Clean up alloc	ated array and buffers */
	for (int i=0; i<nattrs; i++) {
		glDisableVertexAttribArray(i);
	}
	glDeleteBuffers(nattrs, vbo);
	glDeleteVertexArrays(1, &vao);
	return 1;
}

/** Initialize shape type */
void shape_init(Value th) {
	// Shape is a new Region
	pushSym(th, "new");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	Value Shape = getFromTop(th, 0);
		pushCMethod(th, shape_new);
		popProperty(th, 0, "new");
		pushCMethod(th, shape_renderit);
		popProperty(th, 0, "_RenderIt");
		pushCMethod(th, shape_sphere);
		popProperty(th, 0, "NewSphere");
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
		popProperty(th, 0, "_$drawPrimitives");
	popGloVar(th, "Shape");
}