/** Array - buffer of numbers
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <math.h>

/** Create a new Buffer value, with number of Xyz structures. */
int xyzs_new(Value th) {
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
		nStructs /= 3;
	}

	// Create the number array
	Value bufv = pushCData(th, pushProperty(th, 0, "newtype"), PegArray, nStructs*3*sizeof(float), sizeof(ArrayHeader));
	ArrayHeader *hdr = (ArrayHeader*) toHeader(bufv);
	hdr->mbrType = PegVec3;
	hdr->structSz = 3;
	hdr->nStructs = nStructs;

	// Fill the number array with floating point numbers converted from ascii
	if (isStr(parm1)) {
		const char *scanp = (const char*) toStr(parm1);
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

/* Append a single Xyz value to the end of the array */
int xyzs_append(Value th) {
	if (getTop(th)<2)
		return 1;
	Value toadd = getLocal(th,1);
	if (isFloat(toadd)) {
		GLfloat afloat = toAfloat(toadd);
		strAppend(th, getLocal(th, 0), (const char*)(&afloat), sizeof(GLfloat));
	}
	//else if (isxStr(toadd))
	//	strAppend(th, getLocal(th, 0), toHeader(toadd), getxSize(toadd));
	return 1;
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
	Value bufv = pushCData(th, pushProperty(th, 0, "newtype"), PegArray, nStructs*4*sizeof(float), sizeof(ArrayHeader));
	ArrayHeader *hdr = (ArrayHeader*) toHeader(bufv);
	hdr->mbrType = PegVec4;
	hdr->structSz = 4;
	hdr->nStructs = nStructs;

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
	//else if (isxStr(toadd))
	//	strAppend(th, getLocal(th, 0), toxStr(toadd), getxSize(toadd));
	return 1;
}

void array_init(Value th) {
	pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, xyzs_append);
			popProperty(th, 1, "<<");
		popProperty(th, 0, "newtype");
		pushCMethod(th, xyzs_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Xyzs");

	pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, colors_append);
			popProperty(th, 1, "<<");
		popProperty(th, 0, "newtype");
		pushCMethod(th, colors_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Colors");
}