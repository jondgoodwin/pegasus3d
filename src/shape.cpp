/** Shape type - Handles a 3-D geometric mesh
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/
#include "pegasus3d.h"
#include "xyzmath.h"
#include <math.h>

/** Generate a sphere shape centered at (0,0,0), passing radius and nsegments.
	The geometry is via longitude and latitude divisions. 
	Normals extend outwards. 
	The top point's uv is (0,1) with u wrapping horizontally around the latitudes. */
int shape_sphere(Value th) {
	// Obtain and validate parameters
	float radius = getTop(th)>=2 && isFloat(getLocal(th, 1))? toAfloat(getLocal(th,1)) : 1.0f;
	int nsegments = getTop(th)>=3 && isInt(getLocal(th, 2))? toAint(getLocal(th,2)) : 15;
	int nverts = (nsegments+1) * (nsegments-2) + 2;
	int nindices = 6*nsegments*nsegments;
	if (nsegments<=2 || nverts>=65536) {
		pushValue(th, aNull);
		return 1;
	}

	// Push a new Shape on the stack and give it buffer properties
	int sphereidx = getTop(th);
	pushSym(th, "New");
	pushGloVar(th, "Shape");
	getCall(th, 1, 1);
	pushSym(th, "New");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value posval = getFromTop(th, 0);
	popProperty(th, sphereidx, "positions");
	pushSym(th, "New");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value normval = getFromTop(th, 0);
	popProperty(th, sphereidx, "normals");
	pushSym(th, "New");
	pushGloVar(th, "Uvs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value uvval = getFromTop(th, 0);
	popProperty(th, sphereidx, "uvs");
	pushSym(th, "New");
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
	strAppend(th, uvval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, uvval, (const char*)(&one), sizeof(GLfloat));

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
			GLfloat uv1 = (GLfloat)j/(GLfloat)nsegments;
			GLfloat uv2 = (GLfloat)(nsegments-i)/(GLfloat)nsegments;
			strAppend(th, uvval, (const char*)(&uv1), sizeof(GLfloat));
			strAppend(th, uvval, (const char*)(&uv2), sizeof(GLfloat));

			// Generate two triangles per segment
			if (i==1) {
				v0 = 0;
				v1 = j+2;
				v2 = j+1;
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v1), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
			} else {
				v0 = ((i-2)*nsegments)+j+1;
				v1 = ((i-2)*nsegments)+(j+2);
				v2 = ((i-1)*nsegments)+(j+2);
				v3 = ((i-1)*nsegments)+j+1;
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v3), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v1), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
			}
		}
		GLfloat x = -sin(mang);
		GLfloat y = cos(mang);
		GLfloat z = 0.0f;
		strAppend(th, normval, (const char*)(&x), sizeof(GLfloat));
		strAppend(th, normval, (const char*)(&y), sizeof(GLfloat));
		strAppend(th, normval, (const char*)(&z), sizeof(GLfloat));
		x *= radius;
		y *= radius;
		z *= radius;
		strAppend(th, posval, (const char*)(&x), sizeof(GLfloat));
		strAppend(th, posval, (const char*)(&y), sizeof(GLfloat));
		strAppend(th, posval, (const char*)(&z), sizeof(GLfloat));
		GLfloat uv1 = 1.0f;
		GLfloat uv2 = (GLfloat)(nsegments-i)/(GLfloat)nsegments;
		strAppend(th, uvval, (const char*)(&uv1), sizeof(GLfloat));
		strAppend(th, uvval, (const char*)(&uv2), sizeof(GLfloat));
	}

	// Bottom pole vertex
	strAppend(th, uvval, (const char*)(&zero), sizeof(GLfloat));
	strAppend(th, uvval, (const char*)(&zero), sizeof(GLfloat));
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

/** Generate a plane surface shape centered on (0,0,0), passing nsegments.
	The plane is flat to y, and extends between -1 and 1 in x and z.
	The plane normal is positive y. 
	The uv coordinate of 0,0 is the corner at (-1, 0, 1) */
int shape_plane(Value th) {
	// Obtain and validate parameters
	int nsegments = getTop(th)>=2 && isInt(getLocal(th, 1))? toAint(getLocal(th,1)) : 1;
	GLfloat uvrepeat = getTop(th)>=3 && isFloat(getLocal(th, 2))? toAfloat(getLocal(th,2)) : 1.0f;
	int nverts = (nsegments+1) * (nsegments+1);
	int nindices = 6*nsegments*nsegments;
	if (nsegments<1 || nverts>=65536) {
		pushValue(th, aNull);
		return 1;
	}

	// Push a new Shape on the stack and give it buffer properties
	int planeidx = getTop(th);
	pushSym(th, "New");
	pushGloVar(th, "Shape");
	getCall(th, 1, 1);
	pushSym(th, "New");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value posval = getFromTop(th, 0);
	popProperty(th, planeidx, "positions");
	pushSym(th, "New");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value normval = getFromTop(th, 0);
	popProperty(th, planeidx, "normals");
	pushSym(th, "New");
	pushGloVar(th, "Uvs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value uvval = getFromTop(th, 0);
	popProperty(th, planeidx, "uvs");
	pushSym(th, "New");
	pushGloVar(th, "Integers");
	pushValue(th, anInt(nindices));
	getCall(th, 2, 1);
	Value indxval = getFromTop(th, 0);
	popProperty(th, planeidx, "indices");

    // each vertex,  vertically and then horizontally
	int v0, v1, v2, v3;
	GLfloat zero = 0.0f;
	GLfloat one = 1.0f;
	for (int segz=0; segz<=nsegments; segz++) {
		for (int segx=0; segx<=nsegments; segx++) {
			GLfloat x = 2.0f * (GLfloat)segx/(GLfloat)nsegments - 1.0f;
			GLfloat z = 2.0f * (GLfloat)segz/(GLfloat)nsegments - 1.0f;
			strAppend(th, posval, (const char*)(&x), sizeof(GLfloat));
			strAppend(th, posval, (const char*)(&zero), sizeof(GLfloat));
			strAppend(th, posval, (const char*)(&z), sizeof(GLfloat));
			strAppend(th, normval, (const char*)(&zero), sizeof(GLfloat));
			strAppend(th, normval, (const char*)(&one), sizeof(GLfloat));
			strAppend(th, normval, (const char*)(&zero), sizeof(GLfloat));
			GLfloat uv1 = uvrepeat * (GLfloat)segx/(GLfloat)nsegments;
			GLfloat uv2 = uvrepeat * (1.0f - (GLfloat)segz/(GLfloat)nsegments);
			strAppend(th, uvval, (const char*)(&uv1), sizeof(GLfloat));
			strAppend(th, uvval, (const char*)(&uv2), sizeof(GLfloat));

			// Generate two triangles per segment
			if (segx>0 && segz>0) {
				v0 = ((segz-1)*(nsegments+1))+segx-1;
				v1 = ((segz-1)*(nsegments+1))+segx;
				v2 = (segz*(nsegments+1))+segx;
				v3 = (segz*(nsegments+1))+segx-1;
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v3), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v0), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v1), sizeof(unsigned short));
				strAppend(th, indxval, (const char*)(&v2), sizeof(unsigned short));
			}
		}
	}
	return 1;
}

const GLfloat cube_positions[] = {
	-1., 1., -1.,  1., 1., -1.,  1., 1., 1.,  -1., 1., 1.,
	-1., -1., -1.,  1., -1., -1.,  1., -1., 1.,  -1., -1., 1.,
	-1., 1., -1.,  1., 1., -1.,  1., -1., -1.,  -1., -1., -1.,
	-1., 1., 1.,  1., 1., 1.,  1., -1., 1.,  -1., -1., 1.,
	-1., -1., -1.,  -1., 1., -1.,  -1., 1., 1.,  -1., -1., 1.,
	1., -1., -1.,  1., 1., -1.,  1., 1., 1.,  1., -1., 1.
};
const char cube_indices[] = 
	"0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23";

/** Generate a cube centered at 0.0.0. Parameter specifies its size. It has no uvs or normals. */
int shape_cube(Value th) {
	// Obtain and validate parameters
	GLfloat cubesize = getTop(th)>=2 && isFloat(getLocal(th, 1))? toAfloat(getLocal(th,1)) : 1.0f;
	int nverts = 72;

	// Push a new Shape on the stack and give it buffer properties
	int cubeidx = getTop(th);
	pushSym(th, "New");
	pushGloVar(th, "Shape");
	getCall(th, 1, 1);
	pushSym(th, "New");
	pushGloVar(th, "Xyzs");
	pushValue(th, anInt(nverts));
	getCall(th, 2, 1);
	Value posval = getFromTop(th, 0);
	popProperty(th, cubeidx, "positions");
	pushSym(th, "New");
	pushGloVar(th, "Integers");
	pushString(th, aNull, cube_indices);
	getCall(th, 2, 1);
	popProperty(th, cubeidx, "indices");

	for (int v = 0; v<nverts; v++) {
		GLfloat pos = cubesize * cube_positions[v];
		strAppend(th, posval, (const char*)(&pos), sizeof(GLfloat));
	}

	return 1;
}

/** Get the draw property (symbol) */
int shape_getDraw(Value th) {
	pushProperty(th, 0, "_drawsym");
	return 1;
}

/** Set the draw property's closure value using symbols */
int shape_setDraw(Value th) {
	GLint draw = GL_TRIANGLES;
	Value drawsym = getLocal(th, 1);
	if (isSym(drawsym)) {
		const char *drawstr = toStr(drawsym);
		if (strcmp("Triangles", drawstr)==0) draw=GL_TRIANGLES;
		else if (strcmp("TriangleStrip", drawstr)==0) draw=GL_TRIANGLE_STRIP;
		else if (strcmp("TriangleFan", drawstr)==0) draw=GL_TRIANGLE_FAN;
		else if (strcmp("TrianglesAdjacency", drawstr)==0) draw=GL_TRIANGLES_ADJACENCY;
		else if (strcmp("TriangleStripAdjacency", drawstr)==0) draw=GL_TRIANGLE_STRIP_ADJACENCY;
		else if (strcmp("Patches", drawstr)==0) draw=GL_PATCHES;
		else if (strcmp("Lines", drawstr)==0) draw=GL_LINES;
		else if (strcmp("LineStrip", drawstr)==0) draw=GL_LINE_STRIP;
		else if (strcmp("LineLoop", drawstr)==0) draw=GL_LINE_LOOP;
		else if (strcmp("LinesAdjacency", drawstr)==0) draw=GL_LINES_ADJACENCY;
		else if (strcmp("LineStripAdjacency", drawstr)==0) draw=GL_LINE_STRIP_ADJACENCY;
		else if (strcmp("Points", drawstr)==0) draw=GL_POINTS;
		else if (strcmp("Polygon", drawstr)==0) draw=GL_POLYGON; // TriangleFan
	}
	pushValue(th, anInt(draw));
	popProperty(th, 0, "_draw");
	pushValue(th, drawsym);
	popProperty(th, 0, "_drawsym");
	return 0;
}

/** Create a new shape */
int shape_new(Value th) {
	pushSym(th, "New");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	setType(th, getFromTop(th, 0), getLocal(th, 0));
	return 1;
}

/** Render the shape */
int shape_renderit(Value th) {
	int selfidx = 0;
	int contextidx = 1;

	// Render the shader, loading it and its uniforms
	pushSym(th, "_Render");
	Value shader = pushProperty(th, selfidx, "shader");
	if (shader == aNull) {
		popValue(th);
		shader = pushProperty(th, contextidx, "shader");
	}
	pushLocal(th, contextidx);
	pushLocal(th, selfidx);
	getCall(th, 3, 0);

	// Turn on blending for shapes that use translucent colors
	Value transparent = pushProperty(th, selfidx, "transparent");
	popValue(th);
	if (!isFalse(transparent)) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw the vertexes using the vertex attribute buffers
	GLuint vao;
	GLuint vbo[100];
	unsigned int nverts = -1;

	// Get the list of vertex attributes
	Value vertattsym = pushSym(th, "attributes");
	Value vertattrlistv = getProperty(th, shader, vertattsym);
	popValue(th); // symbol
	int nattrs = getSize(vertattrlistv);

	// Set up Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Allocate and assign as many Vertex Buffer Objects to our handle as we have attributes
	// then load and activate each one. 
	glGenBuffers(nattrs, vbo);
	Value attrsource = getLocal(th, selfidx);
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

	// How shall we draw the primitives?
	Value drawprop = pushGetActProp(th, selfidx, "_draw");
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
		glDeleteBuffers(1, &elementbuffer);
	}
	/* Otherwise, draw specified primitives using vertices defined by attribute buffers */
	else
		glDrawArrays(drawmode, 0, nverts);
	popValue(th); // vertices

	/* Clean up allocated array and buffers */
	for (int i=0; i<nattrs; i++) {
		glDisableVertexAttribArray(i);
	}
	glDeleteBuffers(nattrs, vbo);
	glDeleteVertexArrays(1, &vao);

	if (!isFalse(transparent))
		glDisable(GL_BLEND);

	return 1;
}

/** Initialize shape type */
void shape_init(Value th) {
	// Shape is a new Region
	pushSym(th, "New");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	Value Shape = getFromTop(th, 0);
		pushSym(th, "Shape");
		popProperty(th, 0, "_name");
		pushCMethod(th, shape_new);
		popProperty(th, 0, "New");
		pushCMethod(th, shape_renderit);
		popProperty(th, 0, "_RenderIt");
		pushCMethod(th, shape_sphere);
		popProperty(th, 0, "NewSphere");
		pushCMethod(th, shape_plane);
		popProperty(th, 0, "NewPlane");
		pushCMethod(th, shape_cube);
		popProperty(th, 0, "NewCube");

		pushCMethod(th, shape_getDraw);
		pushCMethod(th, shape_setDraw);
		pushClosure(th, 2);
		popProperty(th, 0, "draw");
	popGloVar(th, "Shape");
}