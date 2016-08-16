/** Group definition and rendering
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new group */
int group_new(Value th) {
	pushType(th, getLocal(th, 0), 8); // Inherit from creating prototype
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
			getCall(th, 2, 0);
		}
	}

	return 0;
}

/** Initialize Group type */
void group_init(Value th) {
	pushType(th, aNull, 16);
		pushCMethod(th, group_new);
		popProperty(th, 0, "new");
		pushCMethod(th, shared_render);
		popProperty(th, 0, "_Render");
		pushCMethod(th, group_renderit);
		popProperty(th, 0, "_RenderIt");
	popGloVar(th, "Group");
}
