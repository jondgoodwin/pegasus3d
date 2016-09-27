/** Xyz vector or coordinate
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"
#include <float.h>

/** Create a new Xyz value. Its xyz components are initialized by another Xyz or up to 3 Floats as parameters.
  otherwise they are initialized to 0.0 (the zero vector). */
int xyz_new(Value th) {
	Value xyzv = pushCData(th, pushProperty(th, 0, "_newtype"), PegVec3, 0, sizeof(Xyz));
	Xyz *xyz = (Xyz*) toHeader(xyzv);
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3)) {
		Xyz *other = (Xyz*) toHeader(getLocal(th,1));
		xyz->x = other->x;
		xyz->y = other->y;
		xyz->z = other->z;
	}
	else {
		xyz->x = getTop(th)>1 && isFloat(getLocal(th,1))? toAfloat(getLocal(th, 1)) : 0.0f;
		xyz->y = getTop(th)>2 && isFloat(getLocal(th,2))? toAfloat(getLocal(th, 2)) : 0.0f;
		xyz->z = getTop(th)>3 && isFloat(getLocal(th,3))? toAfloat(getLocal(th, 3)) : 0.0f;
	}
	return 1;
}

/** Set xyz components to 0.0 */
int xyz_zero(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	xyz->x = xyz->y = xyz->z = 0.0;
	setTop(th, 1);
	return 1;
}

/** Update Xyz value's x,y,z with components from passed Xyz or passed x,y,z Floats */
int xyz_set(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && !isCDataType(getLocal(th,1),PegVec3)) {
		Xyz *other = (Xyz*) toHeader(getLocal(th,1));
		xyz->x = other->x;
		xyz->y = other->y;
		xyz->z = other->z;
	}
	else {
		if (getTop(th)>1 && isFloat(getLocal(th,1)))
			xyz->x = toAfloat(getLocal(th, 1));
		if (getTop(th)>2 && isFloat(getLocal(th,2)))
			xyz->y = toAfloat(getLocal(th, 2));
		if (getTop(th)>3 && isFloat(getLocal(th,3)))
			xyz->z = toAfloat(getLocal(th, 3));
	}
	setTop(th, 1);
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

/** Get x component */
int xyz_xget(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyz->x));
	return 1;
}
/** Set x component */
int xyz_xset(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		xyz->x = toAfloat(getLocal(th,1));
	return 0;
}

/** Get y component */
int xyz_yget(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyz->y));
	return 1;
}
/** Set y component */
int xyz_yset(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		xyz->y = toAfloat(getLocal(th,1));
	return 0;
}

/** Get z component */
int xyz_zget(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyz->z));
	return 1;
}
/** Set z component */
int xyz_zset(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		xyz->z = toAfloat(getLocal(th,1));
	return 0;
}

bool almostequal(Afloat a, Afloat b) {
	Afloat diff;
	if ((diff = fabs(a - b))<0.0000001)
		return true;
    a = fabs(a);
    b = fabs(b);
    Afloat largest = (b > a) ? b : a;
	return (diff <= largest * FLT_EPSILON);
}

/** Compare self to another Xyz value. Return true if equal, false if not equal, null otherwise. */
int xyz_isequal(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th,1),PegVec3))
		return 0;
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *other = (Xyz*) toHeader(getLocal(th, 1));
	pushValue(th, almostequal(self->x, other->x) && almostequal(self->y, other->y) && almostequal(self->z, other->z)? aTrue : aFalse);
	return 1;
}

/** Compare self to zero vector. Return true if equal, false if not equal, null otherwise. */
int xyz_iszero(Value th) {
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, almostequal(self->x, 0.0) && almostequal(self->y, 0.0) && almostequal(self->z, 0.0)? aTrue : aFalse);
	return 1;
}

/** Return a float for the length of the Xyz vector */
int xyz_len(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyzLen(xyz)));
	return 1;
}

/** Return a float for the square of the length of the Xyz vector */
int xyz_len2(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(xyz->x*xyz->x + xyz->y*xyz->y + xyz->z*xyz->z));
	return 1;
}

/** Normalize (and return) self's Xyz value to its unit equivalent */
int xyz_normalize(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	xyzNorm(xyz, xyz);
	setTop(th, 1);
	return 1;
}

/** Populate a passed or return a new Xyz value that holds the unit equivalent to self */
int xyz_unit(Value th) {
	Xyz *xyz = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *target;
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3))
		target = (Xyz*) toHeader(getLocal(th, 1));
	else
		target = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "_newtype"), PegVec3, 0, sizeof(Xyz)));
	xyzNorm(target, xyz);
	return 1;
}

/** Return a float representing the distance between two Xyz values. */
int xyz_distance(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3))
		return 0;
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz dist;
	xyzSub(&dist, xyz1, xyz2);
	pushValue(th, aFloat(xyzLen(&dist)));
	return 1;
}

/** Return a float representing the squared distance between two Xyz values. */
int xyz_distance2(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3))
		return 0;
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz dist;
	xyzSub(&dist, xyz1, xyz2);
	pushValue(th, aFloat(dist.x*dist.x + dist.y*dist.y + dist.z*dist.z));
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

/** Negate and return an Xyz value.
	If an Xyz value is passed, put its negative in self.
	Otherwise negate self. */
int xyz_negate(Value th) {
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3)) {
		Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
		self->x = -xyz2->x;
		self->y = -xyz2->y;
		self->z = -xyz2->z;
	} else {
		self->x = -self->x;
		self->y = -self->y;
		self->z = -self->z;
	}
	setTop(th, 1);
	return 1;
}

/** Return the Xyz value after adding two Xyz vectors.
	If three floats are passed, they are added to self.
	If only one Xyz value is passed, it is added to self.
	If two Xyz values are passed, their addition is stored in self.
	If a scalar Float is also passed, it is multiplied to the latter Xyz value. */
 int xyz_add(Value th) {
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>3 && isFloat(getLocal(th,1)) && isFloat(getLocal(th,2)) && isFloat(getLocal(th,3))) {
		self->x += toAfloat(getLocal(th, 1));
		self->y += toAfloat(getLocal(th, 2));
		self->z += toAfloat(getLocal(th, 3));
	}
	else if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3)) {
		Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
		if (getTop(th)>2 && isCDataType(getLocal(th, 2), PegVec3)) {
			Xyz *xyz3 = (Xyz*) toHeader(getLocal(th, 2));
			if (isFloat(getLocal(th, 3))) {
				Afloat dt = toAfloat(getLocal(th, 3));
				self->x = xyz2->x + dt * xyz3->x;
				self->y = xyz2->y + dt * xyz3->y;
				self->z = xyz2->z + dt * xyz3->z;
			}
			else {
				self->x = xyz2->x + xyz3->x; self->y = xyz2->y + xyz3->y; self->z = xyz2->z + xyz3->z;
			}
		}
		else if (isFloat(getLocal(th, 2))) {
			Afloat dt = toAfloat(getLocal(th, 2));
			self->x += dt * xyz2->x;
			self->y += dt * xyz2->y;
			self->z += dt * xyz2->z;
		}
		else {
			self->x += xyz2->x; self->y += xyz2->y; self->z += xyz2->z;
		}
	}
	setTop(th, 1);
	return 1;
}

/** Return a new Xyz value that adds two Xyz values */
int xyz_addop(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3)) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "_newtype"), PegVec3, 0, sizeof(Xyz)));
	xyzAdd(newxyz, xyz1, xyz2);
	return 1;
}

/** Return a new Xyz value that subtracts two Xyz values */
int xyz_subtractop(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3)) {
		pushValue(th, aNull);
		return 1;
	}
	Xyz *xyz1 = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "_newtype"), PegVec3, 0, sizeof(Xyz)));
	newxyz->x = xyz1->x - xyz2->x;
	newxyz->y = xyz1->y - xyz2->y;
	newxyz->z = xyz1->z - xyz2->z;
	return 1;
}

/** Return the Xyz value after scaling it up by a scalar or passed x,y,z values. */
int xyz_scale(Value th) {
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	if (getTop(th)>3 && isFloat(getLocal(th,1)) && isFloat(getLocal(th,2)) && isFloat(getLocal(th,3))) {
		self->x *= toAfloat(getLocal(th, 1));
		self->y *= toAfloat(getLocal(th, 2));
		self->z *= toAfloat(getLocal(th, 3));
	}
	else if (getTop(th)>1 && isFloat(getLocal(th, 1))) {
		Afloat scale = toAfloat(getLocal(th, 1));
		self->x *= scale;
		self->y *= scale;
		self->z *= scale;
	}
	else if (getTop(th)>1 && isCDataType(getLocal(th, 1), PegVec3)) {
		Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
		self->x *= xyz2->x; self->y *= xyz2->y; self->z *= xyz2->z;
	}
	setTop(th, 1);
	return 1;
}

/** Return a new Xyz value that is scaled by a float or Xyz value */
int xyz_multop(Value th) {
	if (getTop(th)<2 || !(isFloat(getLocal(th,1)) || isCDataType(getLocal(th, 1), PegVec3)))
		return 0;
	Xyz *newxyz = (Xyz*) toHeader(pushCData(th, pushProperty(th, 0, "_newtype"), PegVec3, 0, sizeof(Xyz)));
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	if (isFloat(getLocal(th,1))) {
		Afloat scale = toAfloat(getLocal(th, 1));
		self->x *= scale;
		self->y *= scale;
		self->z *= scale;
	} else {
		Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
		self->x *= xyz2->x; self->y *= xyz2->y; self->z *= xyz2->z;
	}
	return 1;
}

/** Return the Xyz value after calculating into it the cross-product of two Xyz vectors. */
int xyz_cross(Value th) {
	if (getTop(th)<2 || !isCDataType(getLocal(th, 1), PegVec3))
		return 0;
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	if (getTop(th)>2 && isCDataType(getLocal(th, 2), PegVec3)) {
		Xyz *xyz3 = (Xyz*) toHeader(getLocal(th, 2));
		xyzCross(self, xyz2, xyz3);
	}
	else
		xyzCross(self, self, xyz2);
	setTop(th, 1);
	return 1;
}

/** Calculate and return Xyz value linearly interpolated between two Xyz values by a scalar from 0 to 1 */
int xyz_lerp(Value th) {
	if (getTop(th)<4 || !isCDataType(getLocal(th, 1), PegVec3) || !isCDataType(getLocal(th, 2), PegVec3) || !isFloat(getLocal(th, 3)))
		return 0;
	Xyz *self = (Xyz*) toHeader(getLocal(th, 0));
	Xyz *xyz2 = (Xyz*) toHeader(getLocal(th, 1));
	Xyz *xyz3 = (Xyz*) toHeader(getLocal(th, 2));
	float scalar = toAfloat(getLocal(th, 3));
	self->x = (1.0f - scalar)*xyz2->x + scalar*xyz3->x;
	self->y = (1.0f - scalar)*xyz2->y + scalar*xyz3->y;
	self->z = (1.0f - scalar)*xyz2->z + scalar*xyz3->z;
	return 1;
}

/** Initialize Xyz type and mixin */
void xyz_init(Value th) {
	Value Xyz = pushType(th, aNull, 2);
		pushSym(th, "Xyz");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 32);
			pushSym(th, "*Xyz");
			popProperty(th, 1, "_name");
			pushCMethod(th, xyz_xget);
			pushCMethod(th, xyz_xset);
			pushClosure(th, 2);
			popProperty(th, 1, "x");
			pushCMethod(th, xyz_yget);
			pushCMethod(th, xyz_yset);
			pushClosure(th, 2);
			popProperty(th, 1, "y");
			pushCMethod(th, xyz_zget);
			pushCMethod(th, xyz_zset);
			pushClosure(th, 2);
			popProperty(th, 1, "z");
			pushCMethod(th, xyz_addop);
			popProperty(th, 1, "+");
			pushCMethod(th, xyz_subtractop);
			popProperty(th, 1, "-");
			pushCMethod(th, xyz_multop);
			popProperty(th, 1, "*");
			pushCMethod(th, xyz_add);
			popProperty(th, 1, "Add");
			pushCMethod(th, xyz_distance);
			popProperty(th, 1, "Distance");
			pushCMethod(th, xyz_distance2);
			popProperty(th, 1, "DistanceSquared");
			pushCMethod(th, xyz_dot);
			popProperty(th, 1, "Dot");
			pushCMethod(th, xyz_isequal);
			popProperty(th, 1, "Equals?");
			pushCMethod(th, xyz_len);
			popProperty(th, 1, "Length");
			pushCMethod(th, xyz_len2);
			popProperty(th, 1, "LengthSquared");
			pushCMethod(th, xyz_lerp);
			popProperty(th, 1, "Lerp");
			pushCMethod(th, xyz_negate);
			popProperty(th, 1, "Negate");
			pushCMethod(th, xyz_normalize);
			popProperty(th, 1, "Normalize");
			pushCMethod(th, xyz_scale);
			popProperty(th, 1, "Scale");
			pushCMethod(th, xyz_set);
			popProperty(th, 1, "Set");
			pushCMethod(th, xyz_unit);
			popProperty(th, 1, "Unit");
			pushCMethod(th, xyz_unpack);
			popProperty(th, 1, "Unpack");
			pushCMethod(th, xyz_iszero);
			popProperty(th, 1, "Zero?");
			pushCMethod(th, xyz_zero);
			popProperty(th, 1, "Zero");
		popProperty(th, 0, "_newtype");
		pushCMethod(th, xyz_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Xyz");
}