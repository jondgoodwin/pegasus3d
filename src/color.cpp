/** Color (rgba)
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <math.h>

/** Create a new color value, with passed red, green, blue and alpha values.
  Defaults: White if no passed values, Gray scale if 1 passed value, 1.0 if no alpha. */
int color_new(Value th) {
	Value colorv = pushCData(th, pushProperty(th, 0, "traits"), ColorValue, 0, sizeof(ColorInfo));
	ColorInfo *color = toColor(colorv);
	float defcolor = 1.0f;
	color->red = defcolor = getTop(th)>1 && isFloat(getLocal(th,1))? toAfloat(getLocal(th, 1)) : defcolor;
	color->green = getTop(th)>2 && isFloat(getLocal(th,2))? toAfloat(getLocal(th, 2)) : defcolor;
	color->blue = getTop(th)>3 && isFloat(getLocal(th,3))? toAfloat(getLocal(th, 3)) : defcolor;
	color->alpha = getTop(th)>4 && isFloat(getLocal(th,4))? toAfloat(getLocal(th, 4)) : 1.0f;
	return 1;
}

/** Update color value with passed red, green, blue and alpha values */
int color_set(Value th) {
	ColorInfo *color = toColor(getLocal(th, 0));
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
	ColorInfo *color = toColor(getLocal(th, 0));
	pushValue(th, aFloat(color->red));
	pushValue(th, aFloat(color->green));
	pushValue(th, aFloat(color->blue));
	pushValue(th, aFloat(color->alpha));
	return 4;
}

/** Initialize Color type and mixin */
void color_init(Value th) {
	Value Camera = pushType(th, aNull, 2);
		pushSym(th, "Color");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushSym(th, "*Color");
			popProperty(th, 1, "_name");
			pushCMethod(th, color_set);
			popProperty(th, 1, "set");
			pushCMethod(th, color_unpack);
			popProperty(th, 1, "unpack");
		popProperty(th, 0, "traits");
		pushCMethod(th, color_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Color");
}
