/** Color (rgba)
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <math.h>

struct ColorInfo {
	float red;
	float green;
	float blue;
	float alpha;
};

/** Create a new color value, with passed red, green, blue and alpha values.
  Defaults: White if no passed values, Gray scale if 1 passed value, 1.0 if no alpha. */
int color_new(Value th) {
	Value colorv = pushCData(th, pushProperty(th, 0, "newtype"), PegVec4, 0, sizeof(ColorInfo));
	ColorInfo *color = (struct ColorInfo*) toHeader(colorv);
	float default = 1.0f;
	color->red = default = getTop(th)>1 && isFloat(getLocal(th,1))? toAfloat(getLocal(th, 1)) : default;
	color->green = getTop(th)>2 && isFloat(getLocal(th,2))? toAfloat(getLocal(th, 2)) : default;
	color->blue = getTop(th)>3 && isFloat(getLocal(th,3))? toAfloat(getLocal(th, 3)) : default;
	color->alpha = getTop(th)>4 && isFloat(getLocal(th,4))? toAfloat(getLocal(th, 4)) : 1.0f;
	return 1;
}

/** Update color value with passed red, green, blue and alpha values */
int color_set(Value th) {
	ColorInfo *color = (struct ColorInfo*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && isFloat(getLocal(th,1)))
		color->red = toAfloat(getLocal(th, 1));
	if (getTop(th)>2 && isFloat(getLocal(th,2)))
		color->green = toAfloat(getLocal(th, 2));
	if (getTop(th)>3 && isFloat(getLocal(th,3)))
		color->blue = toAfloat(getLocal(th, 3));
	if (getTop(th)>4 && isFloat(getLocal(th,4)))
		color->alpha = toAfloat(getLocal(th, 4));
	return 1;
}

/** Return 4 values: red, green, blue and alpha */
int color_unpack(Value th) {
	ColorInfo *color = (struct ColorInfo*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(color->red));
	pushValue(th, aFloat(color->green));
	pushValue(th, aFloat(color->blue));
	pushValue(th, aFloat(color->alpha));
	return 4;
}

/** Initialize Color type and mixin */
void color_init(Value th) {
	Value Camera = pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, color_set);
			popProperty(th, 1, "set");
			pushCMethod(th, color_unpack);
			popProperty(th, 1, "unpack");
		popProperty(th, 0, "newtype");
		pushCMethod(th, color_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Color");
}