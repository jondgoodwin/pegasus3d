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
int group_render(Value th) {
	int selfidx = 0;
	int contextidx = 1;
	if (getTop(th)<2)
		return 0; // Context must be passed

	// Create new context that inherits from old
	int newcontextidx = getTop(th);
	pushSym(th, "new");
	pushLocal(th, contextidx);
	getCall(th, 1, 1);
	Value s = getLocal(th, newcontextidx);

	// Ask if we should render it? (and augment context)
	pushProperty(th, selfidx, "render?"); // Assume it is method
	pushLocal(th, newcontextidx); // make new context self to render? method
	getCall(th, 1, 1);
	Value torender = popValue(th);
	if (torender!=aFalse) {

		// Render every part in group, in order
		Value parts = pushProperty(th, selfidx, "parts");
		if (isArr(parts)) {
			Aint sz = getSize(parts);
			for (Aint i=0; i<sz; i++) {
				pushSym(th, "_Render");
				pushValue(th, arrGet(th, parts, i));
				pushLocal(th, newcontextidx);
				getCall(th, 2, 0);
			}
		}
	}

	return 0;
}

/** Initialize Group type */
void scene_init(Value th) {
	pushType(th, aNull, 16);
		pushCMethod(th, group_new);
		popProperty(th, 0, "new");
		pushCMethod(th, group_render);
		popProperty(th, 0, "_Render");
	popGloVar(th, "Group");
}
