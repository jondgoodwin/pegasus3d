/** Light type - Handles a 3-D light source
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/
#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new light */
int light_new(Value th) {
	pushType(th, getLocal(th, 0), 16);
	return 1;
}

/** Prepare a light for rendering */
int light_renderprep(Value th) {
	int selfidx = 0;
	int cameraidx = 1;

	// Gather properties we need to calculate
	Value mmatrixv = pushProperty(th, selfidx, "mmatrix"); popValue(th);
	if (isCDataType(mmatrixv, PegMat4)) {

		// Obtain storage spot to put camera coordinates for light's origin
		Value lightoriginv = pushProperty(th, cameraidx, "lightOrigin");
		if (lightoriginv = aNull) {
			pushSym(th, "New");
			pushGloVar(th, "Xyz");
			getCall(th, 1, 1);
			lightoriginv = getFromTop(th, 0);
			popProperty(th, cameraidx, "lightOrigin");
		}

		// Calculate camera's coordinates for light's origin
		Xyz *lightorigin = (Xyz*) toHeader(lightoriginv);
		Mat4 *mmat = (Mat4*) toHeader(mmatrixv);
		lightorigin->x = (*mmat)[12];
		lightorigin->y = (*mmat)[13];
		lightorigin->z = (*mmat)[14];
	}
	pushProperty(th, selfidx, "ambient");
	popProperty(th, cameraidx, "ambient");
	pushProperty(th, selfidx, "lightColor");
	popProperty(th, cameraidx, "lightColor");

	return 0;
}

/** Initialize light type */
void light_init(Value th) {
	Value Light = pushType(th, aNull, 16);
		Value Placement = pushGloVar(th, "Placement"); popValue(th);
		addMixin(th, Light, Placement);
		pushSym(th, "Light");
		popProperty(th, 0, "_name");
		pushCMethod(th, light_new);
		popProperty(th, 0, "New");
		pushCMethod(th, light_renderprep);
		popProperty(th, 0, "_RenderPrep");
	popGloVar(th, "Light");
}
