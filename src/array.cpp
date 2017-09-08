/** Array - buffer of numbers
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <math.h>
#include <stdlib.h>

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
	Value bufv = pushCData(th, pushProperty(th, 0, "traits"), ArrayValue, nStructs*3*sizeof(float), sizeof(ArrayHeader));
	ArrayHeader *hdr = toArrayHeader(bufv);
	hdr->mbrType = XyzValue;
	hdr->structSz = 3;
	hdr->nStructs = nStructs;

	// Fill the number array with floating point numbers converted from ascii
	if (isStr(parm1)) {
		char *scanp = (char*)toStr(parm1);
		while (*scanp) {
			if (*scanp=='-' || (*scanp>='0' && *scanp<='9')) {
				GLfloat afloat = (GLfloat) strtod(scanp, &scanp);
				strAppend(th, bufv, (const char*)(&afloat), sizeof(GLfloat));
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

int xyzs_getx(Value th) {
	if (getTop(th)<2 || !isInt(getLocal(th, 1)))
		return 0;
	pushValue(th, aFloat(((float *)toCData(getLocal(th, 0)))[3*toAint(getLocal(th, 1))]));
	return 1;
}

int xyzs_setx(Value th) {
	if (getTop(th)<3 || !isInt(getLocal(th, 2))  || !isFloat(getLocal(th, 1)))
		return 0;
	((float *)toCData(getLocal(th, 0)))[3*toAint(getLocal(th, 1))] = toAfloat(getLocal(th, 1));
	return 0;
}

int xyzs_gety(Value th) {
	if (getTop(th)<2 || !isInt(getLocal(th, 1)))
		return 0;
	pushValue(th, aFloat(((float *)toCData(getLocal(th, 0)))[1+3*toAint(getLocal(th, 1))]));
	return 1;
}

int xyzs_sety(Value th) {
	int mmm = 1+3*toAint(getLocal(th, 2));
	float nnn = toAfloat(getLocal(th, 1));
	if (getTop(th)<3 || !isInt(getLocal(th, 2))  || !isFloat(getLocal(th, 1)))
		return 0;
	((float *)toCData(getLocal(th, 0)))[1+3*toAint(getLocal(th, 2))] = toAfloat(getLocal(th, 1));
	return 0;
}

int xyzs_getz(Value th) {
	if (getTop(th)<2 || !isInt(getLocal(th, 1)))
		return 0;
	pushValue(th, aFloat(((float *)toCData(getLocal(th, 0)))[2+3*toAint(getLocal(th, 1))]));
	return 1;
}

int xyzs_setz(Value th) {
	if (getTop(th)<3 || !isInt(getLocal(th, 2))  || !isFloat(getLocal(th, 1)))
		return 0;
	((float *)toCData(getLocal(th, 0)))[2+3*toAint(getLocal(th, 1))] = toAfloat(getLocal(th, 1));
	return 0;
}

/** Create a new Buffer value, with number of Uv structures. */
int uvs_new(Value th) {
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
		nStructs /= 2;
	}

	// Create the number array
	Value bufv = pushCData(th, pushProperty(th, 0, "traits"), ArrayValue, nStructs*3*sizeof(float), sizeof(ArrayHeader));
	ArrayHeader *hdr = toArrayHeader(bufv);
	hdr->mbrType = XyzValue;
	hdr->structSz = 2;
	hdr->nStructs = nStructs;

	// Fill the array with floating point numbers converted from ascii
	if (isStr(parm1)) {
		char *scanp = (char*) toStr(parm1);
		while (*scanp) {
			if (*scanp=='-' || (*scanp>='0' && *scanp<='9')) {
				GLfloat afloat = (GLfloat) strtod(scanp, &scanp);
				strAppend(th, bufv, (const char*)(&afloat), sizeof(GLfloat));
			}
			else
				scanp++;
		}
	}
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
	Value bufv = pushCData(th, pushProperty(th, 0, "traits"), ArrayValue, nStructs*4*sizeof(float), sizeof(ArrayHeader));
	ArrayHeader *hdr = toArrayHeader(bufv);
	hdr->mbrType = ColorValue;
	hdr->structSz = 4;
	hdr->nStructs = nStructs;

	// Fill the number array with floating point numbers converted from ascii
	if (isStr(parm1)) {
		char *scanp = (char*)toStr(parm1);
		while (*scanp) { 
			if (*scanp=='-' || (*scanp>='0' && *scanp<='9')) {
				GLfloat afloat = (GLfloat) strtod(scanp, &scanp);
				strAppend(th, bufv, (const char*)(&afloat), sizeof(GLfloat));
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
		pushSym(th, "Xyzs");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushSym(th, "*Xyzs");
			popProperty(th, 1, "_name");
			pushCMethod(th, xyzs_append);
			popProperty(th, 1, "<<");
			pushCMethod(th, xyzs_getx);
			pushCMethod(th, xyzs_setx);
			pushClosure(th, 2);
			popProperty(th, 1, "x");
			pushCMethod(th, xyzs_gety);
			pushCMethod(th, xyzs_sety);
			pushClosure(th, 2);
			popProperty(th, 1, "y");
			pushCMethod(th, xyzs_getz);
			pushCMethod(th, xyzs_setz);
			pushClosure(th, 2);
			popProperty(th, 1, "z");
		popProperty(th, 0, "traits");
		pushCMethod(th, xyzs_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Xyzs");

	pushType(th, aNull, 2);
		pushSym(th, "Uvs");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushSym(th, "*Uvs");
			popProperty(th, 1, "_name");
			//pushCMethod(th, uvs_append);
			//popProperty(th, 1, "<<");
		popProperty(th, 0, "traits");
		pushCMethod(th, uvs_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Uvs");

	pushType(th, aNull, 2);
		pushSym(th, "Colors");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushSym(th, "*Colors");
			popProperty(th, 1, "_name");
			pushCMethod(th, colors_append);
			popProperty(th, 1, "<<");
		popProperty(th, 0, "traits");
		pushCMethod(th, colors_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Colors");
}