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
	Value colorv = pushNumbers(th, pushProperty(th, 0, "newtype"), 1, 4, sizeof(GLfloat), false, false, 0);
	ColorInfo *color = (struct ColorInfo*) toStr(colorv);
	float default = 1.0f;
	color->red = default = getTop(th)>1 && isFloat(getLocal(th,1))? toAfloat(getLocal(th, 1)) : default;
	color->green = getTop(th)>2 && isFloat(getLocal(th,2))? toAfloat(getLocal(th, 2)) : default;
	color->blue = getTop(th)>3 && isFloat(getLocal(th,3))? toAfloat(getLocal(th, 3)) : default;
	color->alpha = getTop(th)>4 && isFloat(getLocal(th,4))? toAfloat(getLocal(th, 4)) : 1.0f;
	return 1;
}

/** Update color value with passed red, green, blue and alpha values */
int color_set(Value th) {
	ColorInfo *color = (struct ColorInfo*) toStr(getLocal(th, 0));
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
	ColorInfo *color = (struct ColorInfo*) toStr(getLocal(th, 0));
	pushValue(th, aFloat(color->red));
	pushValue(th, aFloat(color->green));
	pushValue(th, aFloat(color->blue));
	pushValue(th, aFloat(color->alpha));
	return 4;
}

/** Create a new Buffer value, with number of Xyz structures. */
int colors_new(Value th) {
	// Get nStructs parameter
	if (getTop(th)<2) {
		pushValue(th, aNull);
		return 1;
	}
	Value parm1 = getLocal(th, 1);
	AintIdx nStructs = 0;
	if (isInt(parm1))
		nStructs = toAint(parm1);
	else if (isStr(parm1)) {
		// number of floats is one more than the number of commas
		const char *scanp = toStr(parm1);
		while (*scanp) {
			if (*scanp++==',') 
				nStructs++;
		}
		nStructs++;
		nStructs /= 4;
	}

	// Create the number array
	Value bufv = pushNumbers(th, pushProperty(th, 0, "newtype"), nStructs, 4, sizeof(GLfloat), false, false, 0);

	// Fill the number array with floating point numbers converted from ascii
	if (isStr(parm1)) {
		const char *scanp = toStr(parm1);
		while (*scanp) {
			if ((*scanp>='0' && *scanp<='9')||*scanp=='-') {
				GLfloat afloat = (GLfloat) atof(scanp);
				strAppend(th, bufv, (const char*)(&afloat), sizeof(GLfloat));
				while ((*scanp>='0' && *scanp<='9') || *scanp=='.' || *scanp=='e' || *scanp=='E' || *scanp=='-')
					scanp++;
			}
			else
				scanp++;
		}
	}
	return 1;
}

/* Append a single float or Color value to the end of the array */
int colors_append(Value th) {
	if (getTop(th)<2)
		return 1;
	Value toadd = getLocal(th,1);
	if (isFloat(toadd)) {
		GLfloat afloat = toAfloat(toadd);
		strAppend(th, getLocal(th, 0), (const char*)(&afloat), sizeof(GLfloat));
	}
	else if (isStr(toadd))
		strAppend(th, getLocal(th, 0), toStr(toadd), getSize(toadd));
	return 1;
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

	pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, colors_append);
			popProperty(th, 1, "<<");
		popProperty(th, 0, "newtype");
		pushCMethod(th, colors_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Colors");
}