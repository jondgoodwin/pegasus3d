/** Rect - rectangle defining 2d area
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new Rect */
int rect_new(Value th) {
	Value rectv = pushCData(th, pushProperty(th, 0, "_newtype"), PegRect, 0, sizeof(Rect));
	Rect *rect = (Rect*) toHeader(rectv);
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegRect)) {
		Rect *other = (Rect*) toHeader(getLocal(th,1));
		rect->x = other->x;
		rect->y = other->y;
		rect->h = other->h;
		rect->w = other->w;
	}
	else {
		rect->x = getTop(th)>1 && isInt(getLocal(th,1))? toAint(getLocal(th, 1)) : 0;
		rect->y = getTop(th)>2 && isInt(getLocal(th,2))? toAint(getLocal(th, 2)) : 0;
		rect->h = getTop(th)>3 && isInt(getLocal(th,3))? toAint(getLocal(th, 3)) : 0;
		rect->w = getTop(th)>4 && isInt(getLocal(th,4))? toAint(getLocal(th, 4)) : 0;
	}
	return 1;
}

/** Initialize Quat type and mixin */
void rect_init(Value th) {
	pushType(th, aNull, 4);
		pushSym(th, "Rect");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
		popProperty(th, 0, "_newtype");
		pushCMethod(th, rect_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Rect");
}
