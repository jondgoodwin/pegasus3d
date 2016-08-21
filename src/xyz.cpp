/** Xyz vector or coordinate
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new Xyz value, with passed x,y,z values.
  Defaults: 0.0 where no passed values. */
int xyz_new(Value th) {
	Value xyzv = pushCData(th, pushProperty(th, 0, "newtype"), PegVec3, 0, sizeof(Xyz));
	Xyz *xyz = (Xyz*) toHeader(xyzv);
	xyz->x = getTop(th)>1 && isFloat(getLocal(th,1))? toAfloat(getLocal(th, 1)) : 0.0f;
	xyz->y = getTop(th)>2 && isFloat(getLocal(th,2))? toAfloat(getLocal(th, 2)) : 0.0f;
	xyz->z = getTop(th)>3 && isFloat(getLocal(th,3))? toAfloat(getLocal(th, 3)) : 0.0f;
	return 1;
}

/** Update Xyz value with passed x,y,z values */
int xyz_set(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && isFloat(getLocal(th,1)))
		xyz->x = toAfloat(getLocal(th, 1));
	if (getTop(th)>2 && isFloat(getLocal(th,2)))
		xyz->y = toAfloat(getLocal(th, 2));
	if (getTop(th)>3 && isFloat(getLocal(th,3)))
		xyz->z = toAfloat(getLocal(th, 3));
	return 1;
}

/** Return 3 values: x, y, z */
int xyz_unpack(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyz->x));
	pushValue(th, aFloat(xyz->y));
	pushValue(th, aFloat(xyz->z));
	return 3;
}

/** Return a float for the length of the Xyz vector */
int xyz_len(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyzLen(xyz)));
	return 1;
}

/** Return a new Xyz value that is the unit equivalent to self */
int xyz_norm(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "newtype"), PegVec3, 0, sizeof(Xyz)));
	xyzNorm(newxyz, xyz);
	return 1;
}

/** Return the Xyz value after increasing it using passed x,y,z values.
    One can pass another Xyz or individual x,y,z floats. */
int xyz_addto(Value th) {
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>3) {
		xyz1->x += isFloat(getLocal(th, 1))? toAfloat(getLocal(th, 1)) : 0.0f;
		xyz1->y += isFloat(getLocal(th, 2))? toAfloat(getLocal(th, 2)) : 0.0f;
		xyz1->z += isFloat(getLocal(th, 3))? toAfloat(getLocal(th, 3)) : 0.0f;
	}
	else if (getTop(th)>1 && isCDataType(getLocal(th, 1), PegVec3)) {
		Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
		xyz1->x += xyz2->x; xyz1->y += xyz2->y; xyz1->z += xyz2->z;
	}
	setTop(th, 1);
	return 1;
}

/** Return a new Xyz value that adds two Xyz values */
int xyz_add(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3)) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "newtype"), PegVec3, 0, sizeof(Xyz)));
	xyzAdd(newxyz, xyz1, xyz2);
	return 1;
}

/** Return a new Xyz value that subtracts two Xyz values */
int xyz_subtract(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3)) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "newtype"), PegVec3, 0, sizeof(Xyz)));
	newxyz->x = xyz1->x - xyz2->x;
	newxyz->y = xyz1->y - xyz2->y;
	newxyz->z = xyz1->z - xyz2->z;
	return 1;
}

/** Return a new Xyz value that is the cross product of two Xyz values */
int xyz_cross(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3)) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "newtype"), PegVec3, 0, sizeof(Xyz)));
	xyzCross(newxyz, xyz1, xyz2);
	return 1;
}

/** Return a float representing the dot product of two Xyz values.
	Warning: it does not normalize the Xyz values. */
int xyz_dot(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3)) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	pushValue(th, aFloat(xyzDot(xyz1, xyz2)));
	return 1;
}

/** Initialize Xyz type and mixin */
void xyz_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushMixin(th, aNull, aNull, 16);
			pushCMethod(th, xyz_set);
			popProperty(th, 1, "set");
			pushCMethod(th, xyz_unpack);
			popProperty(th, 1, "unpack");
			pushCMethod(th, xyz_len);
			popProperty(th, 1, "length");
			pushCMethod(th, xyz_norm);
			popProperty(th, 1, "unit");
			pushCMethod(th, xyz_addto);
			popProperty(th, 1, "add!");
			pushCMethod(th, xyz_add);
			popProperty(th, 1, "+");
			pushCMethod(th, xyz_subtract);
			popProperty(th, 1, "-");
			pushCMethod(th, xyz_cross);
			popProperty(th, 1, "*");
			pushCMethod(th, xyz_dot);
			popProperty(th, 1, "dot");
		popProperty(th, 0, "newtype");
		pushCMethod(th, xyz_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Xyz");
}