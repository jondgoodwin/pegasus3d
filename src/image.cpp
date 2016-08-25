/** Image buffer - contents from an image resource
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

	stbi_set_flip_vertically_on_load(true);

	Value contents = getLocal(th,1);
	const char* buffer = toStr(contents);
	int len=getSize(contents);
	int x, y, comp;
	char *image = (char*) stbi_load_from_memory((const stbi_uc *)buffer, len, &x, &y, &comp, 0);
	AuintIdx size = comp * x * y;

	// Allocate buffers and populate header
	Value imagev = pushCData(th, aNull, PegImage, 0, sizeof(ImageHeader));
	ImageHeader *imghdr = (ImageHeader*) toHeader(imagev);
	imghdr->x = x;
	imghdr->y = y;
	imghdr->z = 0;
	imghdr->nbytes = (unsigned char) comp;

	// Give decoded image data to AcornVM instead of copying lots of data into a newly allocated area
	// AcornVM will free it when done, so we should not do so now
	strSwapBuffer(th, imagev, image, size);
	return 1;
}

/** Initialize Image type and plug into Resource */
void image_init(Value th) {
	Value Image = pushType(th, aNull, 2);
		pushSym(th, "Image");
		popProperty(th, 0, "_name");
		pushCMethod(th, image_new);
		popProperty(th, 0, "New");
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