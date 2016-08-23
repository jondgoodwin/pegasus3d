/** Texture
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new texture */
int texture_new(Value th) {
	int newtexidx = getTop(th);
	pushType(th, getLocal(th, 0), 8); // Inherit from creating prototype
	return 1;
}

/** Get a new unused texture unit */
int texture_newUnit(Value th) {
	int texidx = getTop(th);
	pushGloVar(th, "Texture");
	Value nTex = pushProperty(th, texidx, "_nTextures");
	pushValue(th, anInt(1+toAint(nTex)));
	popProperty(th, texidx, "_nTextures");
	return 1;
}

/** Render a texture from a shader's uniform */
int texture_render(Value th) {

	// Return unit number if we have it
	Value texunit = pushProperty(th, 0, "_texUnit");
	if (texunit != aNull)
		return 1;
	popValue(th);

	// Obtain new unit number, which we will return
	pushSym(th, "_NewUnit");
	pushLocal(th, 0);
	getCall(th, 1, 1);
	Value newunit = getFromTop(th, 0);
	pushValue(th, newunit);
	popProperty(th, 0, "_texUnit"); // save it for next use
	glActiveTexture(GL_TEXTURE0 + toAint(newunit));

	// Create texture
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// Copy image data into buffer
	Value image = pushProperty(th, 0, "image");
	ImageHeader *imghdr = (ImageHeader*)toHeader(image);
	int format = imghdr->nbytes>3? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imghdr->x, imghdr->y, 0, format, GL_UNSIGNED_BYTE, toCData(image));
	popValue(th);

	// Edge value sampling
	int wraps = GL_CLAMP_TO_EDGE;
	Value wrapsv = pushProperty(th, 0, "wrapS");
	if (isSym(wrapsv)) {
		const char *wrapstr = toStr(wrapsv);
		if (0==strcmp(wrapstr, "Repeat")) wraps = GL_REPEAT;
		else if (0==strcmp(wrapstr, "MirroredRepeat")) wraps = GL_MIRRORED_REPEAT;
	}
	popValue(th);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps);
	wraps = GL_CLAMP_TO_EDGE;
	wrapsv = pushProperty(th, 0, "wrapT");
	if (isSym(wrapsv)) {
		const char *wrapstr = toStr(wrapsv);
		if (0==strcmp(wrapstr, "Repeat")) wraps = GL_REPEAT;
		else if (0==strcmp(wrapstr, "MirroredRepeat")) wraps = GL_MIRRORED_REPEAT;
	}
	popValue(th);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wraps);

	Value mipmap = pushProperty(th, 0, "mipmap");
	if (mipmap != aFalse)
		glGenerateMipmap(GL_TEXTURE_2D);
	popValue(th);

	// Filter properties
	int filter = GL_LINEAR;
	Value filterv = pushProperty(th, 0, "magFilter");
	if (isSym(filterv)) {
		const char *filterstr = toStr(filterv);
		if (0==strcmp(filterstr, "Nearest")) filter = GL_NEAREST;
	}
	popValue(th);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	filter = GL_NEAREST_MIPMAP_LINEAR;
	filterv = pushProperty(th, 0, "minFilter");
	if (isSym(filterv)) {
		const char *filterstr = toStr(filterv);
		if (0==strcmp(filterstr, "Nearest")) filter = GL_NEAREST;
		else if (0==strcmp(filterstr, "Linear")) filter = GL_LINEAR;
		else if (0==strcmp(filterstr, "NearestMipmapNearest")) filter = GL_NEAREST_MIPMAP_NEAREST;
		else if (0==strcmp(filterstr, "LinearMipmapNearest")) filter = GL_LINEAR_MIPMAP_NEAREST;
		else if (0==strcmp(filterstr, "LinearMipmapLinear")) filter = GL_LINEAR_MIPMAP_LINEAR;
	}
	popValue(th);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	return 1;
}

/** Initialize Texture type */
void texture_init(Value th) {
	Value Image = pushType(th, aNull, 2);
		pushCMethod(th, texture_new);
		popProperty(th, 0, "new");
		pushSym(th, "Texture");
		popProperty(th, 0, "name");
		pushCMethod(th, texture_render);
		popProperty(th, 0, "_Render");
		pushValue(th, anInt(0));
		popProperty(th, 0, "_nTextures");
		pushCMethod(th, texture_newUnit);
		popProperty(th, 0, "_NewUnit");
	popGloVar(th, "Texture");
}