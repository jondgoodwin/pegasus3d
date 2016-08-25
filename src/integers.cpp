/** Integers
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <stdlib.h>

/** Create a new Buffer with number of integers */
int integers_new(Value th) {
	// Default parameter
	if (getTop(th)<2)
		pushValue(th, anInt(32));
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
	}

	// Create the integer array
	Value bufv = pushCData(th, pushProperty(th, 0, "_newtype"), PegArray, nStructs*sizeof(short), sizeof(ArrayHeader));
	ArrayHeader *hdr = (ArrayHeader*) toHeader(bufv);
	hdr->mbrType = PegUint16;
	hdr->structSz = 1;
	hdr->nStructs = nStructs;

	// Fill the array with integers converted from ascii
	if (isStr(parm1)) {
		const char *scanp = toStr(parm1);
		while (*scanp) {
			if ((*scanp>='0' && *scanp<='9')||*scanp=='-') {
				short ival = atoi(scanp);
				strAppend(th, bufv, (const char*)(&ival), sizeof(GLshort));
				while ((*scanp>='0' && *scanp<='9') || *scanp=='.' || *scanp=='e' || *scanp=='E' || *scanp=='-')
					scanp++;
			}
			else
				scanp++;
		}
	}
	return 1;
}

/* Append a single integer to the end of the array */
int integers_append(Value th) {
	if (getTop(th)<2)
		return 1;
	Value toadd = getLocal(th,1);
	if (isInt(toadd)) {
		short ival = (short) toAint(toadd);
		strAppend(th, getLocal(th, 0), (const char*)(&ival), sizeof(GLshort));
	}
	else if (isFloat(toadd)) {
		short ival = (short) toAfloat(toadd);
		strAppend(th, getLocal(th, 0), (const char*)(&ival), sizeof(GLshort));
	}
	return 1;
}

/** Initialize Color type and mixin */
void integers_init(Value th) {
	pushType(th, aNull, 2);
		pushSym(th, "Integers");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushSym(th, "*Integers");
			popProperty(th, 1, "_name");
			pushCMethod(th, integers_append);
			popProperty(th, 1, "<<");
		popProperty(th, 0, "_newtype");
		pushCMethod(th, integers_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Integers");
}