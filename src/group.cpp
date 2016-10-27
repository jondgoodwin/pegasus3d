/** Group definition and rendering
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

/** Create a new group */
int group_new(Value th) {
	pushType(th, getLocal(th, 0), 16);
	return 1;
}

/** Recursively traverse a scene graph's group, preparing it for rendering */
int group_renderprep(Value th) {
	int selfidx = 0;
	int cameraidx = 1;

	Value nodematv = pushProperty(th, selfidx, "mmatrix");

	// Recursively traverse this node's parts
	Value parts = pushProperty(th, selfidx, "parts");
	if (isArr(parts)) {
		Aint sz = getSize(parts);
		for (Aint i=0; i<sz; i++) {
			pushSym(th, "_RenderPrep");
			pushLocal(th, cameraidx);
			pushValue(th, arrGet(th, parts, i));
			pushValue(th, nodematv);
			getCall(th, 3, 0);
		}
	}

	return 0;
}

/** Render group using passed context (parm 1) */
int group_render(Value th) {
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
	Value Group = pushType(th, aNull, 16);
		Value Placement = pushGloVar(th, "Placement"); popValue(th);
		addMixin(th, Group, Placement);
		pushSym(th, "Group");
		popProperty(th, 0, "_name");
		pushCMethod(th, group_new);
		popProperty(th, 0, "New");
		pushCMethod(th, group_render);
		popProperty(th, 0, "_Render");
		pushCMethod(th, group_renderprep);
		popProperty(th, 0, "_RenderPrep");
	popGloVar(th, "Group");
}
