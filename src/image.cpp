/** Image buffer
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/** Create a new image, converting its contents to r/g/b values */
int image_new(Value th) {
	if (getTop(th)<2 || !isStr(getLocal(th,1)))
	{
		pushValue(th, aNull);
		return 1;
	}

	Value contents = getLocal(th,1);
	const char* buffer = toStr(contents);
	int len=getSize(contents);
	int x, y, comp;
	char *image = (char*) stbi_load_from_memory((const stbi_uc *)buffer, len, &x, &y, &comp, 0);

	stbi_image_free(image);
	return 1;
}

/*
GLuint tex[2];
glGenTextures(2, tex);
// glActiveTexture(GL_TEXTURE0); // If using multiple texture units
glBindTexture(GL_TEXTURE_2D, &tex[0]);

// Load texture
// Black/white checkerboard
float pixels[] = {
    0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
};
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

// Wrapping
// GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER (requires a color)
// float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
// glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

// Filtering:  regular or mipmap
// GL_LINEAR, GL_NEAREST, 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// OR do mip-maps for filtering. Generate then filter as above
// GL_NEAREST_MIPMAP_NEAREST: Uses the mipmap that most closely matches the size of the pixel being textured and samples with nearest neighbour interpolation.
// GL_LINEAR_MIPMAP_NEAREST: Samples the closest mipmap with linear interpolation.
// GL_NEAREST_MIPMAP_LINEAR: Uses the two mipmaps that most closely match the size of the pixel being textured and samples with nearest neighbour interpolation.
// GL_LINEAR_MIPMAP_LINEAR: Samples closest two mipmaps with linear interpolation.
glGenerateMipmap(GL_TEXTURE_2D);
// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

// Set uniform to right unit
glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0); // setting uniform for texture unit 0
*/

/** Initialize Color type and mixin */
void image_init(Value th) {
	Value Image = pushType(th, aNull, 2);
		pushCMethod(th, image_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Image");

	// Register this type as Resource's 'acn' extension
	pushGloVar(th, "Resource");
		pushProperty(th, getTop(th) - 1, "extensions");
			pushValue(th, Image);
			popTblSet(th, getTop(th) - 2, "jpg");
			pushValue(th, Image);
			popTblSet(th, getTop(th) - 2, "jpeg");
			pushValue(th, Image);
			popTblSet(th, getTop(th) - 2, "gif");
			pushValue(th, Image);
			popTblSet(th, getTop(th) - 2, "png");
		popValue(th);
	popValue(th);
}