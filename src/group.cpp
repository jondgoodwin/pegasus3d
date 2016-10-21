/** Group definition and rendering
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new group */
int group_new(Value th) {
	pushSym(th, "New");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	setType(th, getFromTop(th, 0), getLocal(th, 0));
	return 1;
}

/** Render group using passed context (parm 1) */
int group_renderit(Value th) {
	int selfidx = 0;
	int contextidx = 1;

	// Render every part in group, in order
	Value parts = pushProperty(th, selfidx, "parts");
	if (isArr(parts)) {
		Aint sz = getSize(parts);
		for (Aint i=0; i<sz; i++) {
			pushSym(th, "_Render");
			pushValue(th, arrGet(th, parts, i));
			pushLocal(th, contextidx);
			pushProperty(th, selfidx, "mmatrix");
			getCall(th, 3, 0);
		}
	}

	return 0;
}

/** Initialize Group type */
void group_init(Value th) {
	// Group is a new Region
	pushSym(th, "New");
	pushGloVar(th, "Region");
	getCall(th, 1, 1);
	Value Group = getFromTop(th, 0);
		pushSym(th, "Group");
		popProperty(th, 0, "_name");
		pushCMethod(th, group_new);
		popProperty(th, 0, "New");
		pushCMethod(th, group_renderit);
		popProperty(th, 0, "_RenderIt");
	popGloVar(th, "Group");
}
