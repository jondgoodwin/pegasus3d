/** Quaternion
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"
#include <float.h>

/** Create a new Quat value. Its xyzw components are initialized by another Xyzw or up to 4 Floats as parameters.
  otherwise they are initialized to 0.0 (the zero vector). */
int quat_new(Value th) {
	Value quatv = pushCData(th, pushProperty(th, 0, "_newtype"), PegVec4, 0, sizeof(Quat));
	Quat *quat = (Quat*) toHeader(quatv);
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec4)) {
		Quat *other = (Quat*) toHeader(getLocal(th,1));
		quat->x = other->x;
		quat->y = other->y;
		quat->z = other->z;
		quat->w = other->w;
	}
	else {
		quat->x = getTop(th)>1 && isFloat(getLocal(th,1))? toAfloat(getLocal(th, 1)) : 0.0f;
		quat->y = getTop(th)>2 && isFloat(getLocal(th,2))? toAfloat(getLocal(th, 2)) : 0.0f;
		quat->z = getTop(th)>3 && isFloat(getLocal(th,3))? toAfloat(getLocal(th, 3)) : 0.0f;
		quat->w = getTop(th)>4 && isFloat(getLocal(th,4))? toAfloat(getLocal(th, 4)) : 1.0f;
	}
	return 1;
}

/** Update Quat value's x,y,z,w with components from passed Xyz or passed x,y,z,w Floats */
int quat_set(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec4)) {
		Quat *other = (Quat*) toHeader(getLocal(th,1));
		quat->x = other->x;
		quat->y = other->y;
		quat->z = other->z;
		quat->w = other->w;
	}
	else {
		if (getTop(th)>1 && isFloat(getLocal(th,1)))
			quat->x = toAfloat(getLocal(th, 1));
		if (getTop(th)>2 && isFloat(getLocal(th,2)))
			quat->y = toAfloat(getLocal(th, 2));
		if (getTop(th)>3 && isFloat(getLocal(th,3)))
			quat->z = toAfloat(getLocal(th, 3));
		if (getTop(th)>4 && isFloat(getLocal(th,4)))
			quat->w = toAfloat(getLocal(th, 4));
	}
	setTop(th, 1);
	return 1;
}

/** Set Quat's x,y,z,w to correspond with float angle and Xyz or floats axis */
int quat_setAxis(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th) < 2 || !isFloat(getLocal(th,1)))
		return 0;
	float angle = toAfloat(getLocal(th, 1));
	float axisx, axisy, axisz;
	if (getTop(th)>2 && isCDataType(getLocal(th,2),PegVec3)) {
		Xyz *axis = (Xyz*) toHeader(getLocal(th,2));
		axisx = axis->x; axisy = axis->y; axisz = axis->z;
	} else if (getTop(th)>4 && isFloat(getLocal(th,2)) && isFloat(getLocal(th,3)) && isFloat(getLocal(th,4))) {
		axisx = toAfloat(getLocal(th,2));
		axisy = toAfloat(getLocal(th,3));
		axisz = toAfloat(getLocal(th,4));
	} else
		return 0;
	float sinangle = sin(angle * 0.5f);
	self->x = axisx * sinangle;
	self->y = axisy * sinangle;
	self->z = axisz * sinangle;
	self->w = cos(angle * 0.5f);
	setTop(th, 1);
	return 1;
}

/** Set Quat's x,y,z,w to correspond with Euler rotations: first X, then Y, then Z */
int quat_setRotateXyz(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	float rotx,roty,rotz;
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3)) {
		Xyz *rots = (Xyz*) toHeader(getLocal(th,1));
		rotx = rots->x; roty = rots->y; rotz = rots->z;
	} else if (getTop(th)>1 && isFloat(getLocal(th,1))) {
		rotx = toAfloat(getLocal(th,1));
		roty = (getTop(th)>2 && isFloat(getLocal(th,2)))? toAfloat(getLocal(th,2)) : 0.0f;
		rotz = (getTop(th)>3 && isFloat(getLocal(th,3)))? toAfloat(getLocal(th,3)) : 0.0f;
	} else
		return 0;
	float cosx = cos(rotx * 0.5f);
	float cosy = cos(roty * 0.5f);
	float cosz = cos(rotz * 0.5f);
	float sinx = sin(rotx * 0.5f);
	float siny = sin(roty * 0.5f);
	float sinz = sin(rotz * 0.5f);
	self->x = sinx * cosy * cosz + cosx * siny * sinz;
	self->y = cosx * siny * cosz - sinx * cosy * sinz;
	self->z = cosx * cosy * sinz + sinx * siny * cosz;
	self->w = cosx * cosy * cosz - sinx * siny * sinz;
	setTop(th, 1);
	return 1;
}

/** Set Quat's x,y,z,w to correspond with Euler rotations: first Y, then X, then Z */
int quat_setRotateYxz(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	float rotx,roty,rotz;
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec3)) {
		Xyz *rots = (Xyz*) toHeader(getLocal(th,1));
		rotx = rots->x; roty = rots->y; rotz = rots->z;
	} else if (getTop(th)>1 && isFloat(getLocal(th,1))) {
		rotx = toAfloat(getLocal(th,1));
		roty = (getTop(th)>2 && isFloat(getLocal(th,2)))? toAfloat(getLocal(th,2)) : 0.0f;
		rotz = (getTop(th)>3 && isFloat(getLocal(th,3)))? toAfloat(getLocal(th,3)) : 0.0f;
	} else
		return 0;
	float cosx = cos(rotx * 0.5f);
	float cosy = cos(roty * 0.5f);
	float cosz = cos(rotz * 0.5f);
	float sinx = sin(rotx * 0.5f);
	float siny = sin(roty * 0.5f);
	float sinz = sin(rotz * 0.5f);
	self->x = sinx * cosy * cosz + cosx * siny * sinz;
	self->y = cosx * siny * cosz - sinx * cosy * sinz;
	self->z = cosx * cosy * sinz - sinx * siny * cosz;
	self->w = cosx * cosy * cosz + sinx * siny * sinz;
	setTop(th, 1);
	return 1;
}

/** Calculate and Set Quat's able to rotate a vector from u vector to v vector */
int quat_setVectors(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th) < 3 || !isCDataType(getLocal(th,1),PegVec3) || !isCDataType(getLocal(th,2),PegVec3))
		return 0;
	Xyz *u = (Xyz*) toHeader(getLocal(th,1));
	Xyz *v = (Xyz*) toHeader(getLocal(th,2));
	Xyz w;
	// Adapted from http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
	float norm_u_norm_v = sqrt(xyzDot(u, u) * xyzDot(v, v));
	float real_part = norm_u_norm_v + xyzDot(u, v);
    if (real_part < 1.e-6f * norm_u_norm_v)
    {
        // If u and v are opposites, rotate 180 degrees around arbitrary axis.
        // Axis normalization happens when we normalize the quaternion.
        real_part = 0.0f;
		if (abs(u->x) > abs(u->z)) {w.x = -u->y; w.y = u->x; w.z = 0.f;}
		else {w.x = 0.f; w.y = -u->z, w.z = u->y;}
    }
    else
		xyzCross(&w, u, v); // Calculate orthogonal axis
	// Normalize and return quarternion
	float inorm2 = 1.0f / sqrt(real_part*real_part + xyzDot(&w, &w));
	self->x = w.x * inorm2;
	self->y = w.y * inorm2;
	self->z = w.z * inorm2;
	self->w = real_part * inorm2;
	setTop(th, 1);
	return 1;
}

/** Get x component */
int quat_xget(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(quat->x));
	return 1;
}
/** Set x component */
int quat_xset(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		quat->x = toAfloat(getLocal(th,1));
	return 0;
}

/** Get y component */
int quat_yget(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(quat->y));
	return 1;
}
/** Set y component */
int quat_yset(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		quat->y = toAfloat(getLocal(th,1));
	return 0;
}

/** Get z component */
int quat_zget(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(quat->z));
	return 1;
}
/** Set z component */
int quat_zset(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		quat->z = toAfloat(getLocal(th,1));
	return 0;
}

/** Get w component */
int quat_wget(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	pushValue(th, aFloat(quat->w));
	return 1;
}
/** Set w component */
int quat_wset(Value th) {
	Quat *quat = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1)
		quat->w = toAfloat(getLocal(th,1));
	return 0;
}

/** Get the conjugate of a Quat value.
	If an Quat value is passed, put its conjugate in self.
	Otherwise do it to self. */
int quat_conjugate(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec4)) {
		Quat *quat2 = (Quat*) toHeader(getLocal(th, 1));
		self->x = -quat2->x;
		self->y = -quat2->y;
		self->z = -quat2->z;
		self->w = quat2->w;
	} else {
		self->x = -self->x;
		self->y = -self->y;
		self->z = -self->z;
		self->w = -self->w;
	}

	setTop(th, 1);
	return 1;
}

/** Normalize and return an Quat value. */
int quat_normalize(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
    float norm2 = sqrt(self->x*self->x + self->y*self->y + self->z*self->z + self->w*self->w);
	if (norm2==0.0f)
		self->x = self->y = self->z = self->w = 0.f;
	else {
		float inorm2 = 1.0f/norm2;
		self->x *= inorm2;
		self->y *= inorm2;
		self->z *= inorm2;
		self->w *= inorm2;
	}
	setTop(th, 1);
	return 1;
}

/** Invert and return an Quat value.
	If an Quat value is passed, put its inverse in self.
	Otherwise invert self. */
int quat_inverse(Value th) {
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	if (getTop(th)>1 && isCDataType(getLocal(th,1),PegVec4)) {
		Quat *quat2 = (Quat*) toHeader(getLocal(th, 1));
		self->x = quat2->x;
		self->y = quat2->y;
		self->z = quat2->z;
		self->w = quat2->w;
	}
	// Conjugate and normalize
    float norm2 = self->x*self->x + self->y*self->y + self->z*self->z + self->w*self->w;
	if (norm2>0.0f) {
		float inorm2 = 1.0f / norm2;
		self->x *= -inorm2;
		self->y *= -inorm2;
		self->z *= -inorm2;
		self->w *= inorm2;
	}
	setTop(th, 1);
	return 1;
}

/** Return a Quat value that is the result of multiplying two quaternions.
	Either self*parm or parm1*parm2. */
int quat_mult(Value th) {
	if (getTop(th)<2 || !(isFloat(getLocal(th,1)) || isCDataType(getLocal(th, 1), PegVec4)))
		return 0;
	Quat *self = (Quat*) toHeader(getLocal(th, 0));
	Quat *quat2 = (Quat*) toHeader(getLocal(th, 1));
	Xyz va, vb, vaxb;
	float wa, wb;
	if (getTop(th)>2) {
		Quat *quat3 = (Quat*) toHeader(getLocal(th, 2));
		va.x = quat2->x;
		va.y = quat2->y;
		va.z = quat2->z;
		vb.x = quat3->x;
		vb.y = quat3->y;
		vb.z = quat3->z;
		wa = quat2->w;
		wb = quat3->w;
	} else {
		va.x = self->x;
		va.y = self->y;
		va.z = self->z;
		vb.x = quat2->x;
		vb.y = quat2->y;
		vb.z = quat2->z;
		wa = self->w;
		wb = quat2->w;
	}
	xyzCross(&vaxb, &va, &vb);
	self->x = wa*vb.x + wb*va.x + vaxb.x;
    self->y = wa*vb.y + wb*va.y + vaxb.y;
    self->z = wa*vb.z + wb*va.z + vaxb.z;
	self->w = wa*wb - xyzDot(&va,&vb);
	setTop(th,1);
	return 1;
}

/** Initialize Quat type and mixin */
void quat_init(Value th) {
	Value Quat = pushType(th, aNull, 4);
		pushSym(th, "Quat");
		popProperty(th, 0, "_name");
		pushMixin(th, aNull, aNull, 16);
			pushSym(th, "*Quat");
			popProperty(th, 1, "_name");
			pushCMethod(th, quat_xget);
			pushCMethod(th, quat_xset);
			pushClosure(th, 2);
			popProperty(th, 1, "x");
			pushCMethod(th, quat_yget);
			pushCMethod(th, quat_yset);
			pushClosure(th, 2);
			popProperty(th, 1, "y");
			pushCMethod(th, quat_zget);
			pushCMethod(th, quat_zset);
			pushClosure(th, 2);
			popProperty(th, 1, "z");
			pushCMethod(th, quat_wget);
			pushCMethod(th, quat_wset);
			pushClosure(th, 2);
			popProperty(th, 1, "w");
			pushCMethod(th, quat_set);
			popProperty(th, 1, "Set");
			pushCMethod(th, quat_setAxis);
			popProperty(th, 1, "SetAngleAxis");
			pushCMethod(th, quat_setRotateXyz);
			popProperty(th, 1, "SetRotateXyz");
			pushCMethod(th, quat_setRotateYxz);
			popProperty(th, 1, "SetRotateYxz");
			pushCMethod(th, quat_conjugate);
			popProperty(th, 1, "Conjugate");
			pushCMethod(th, quat_normalize);
			popProperty(th, 1, "Normalize");
			pushCMethod(th, quat_inverse);
			popProperty(th, 1, "Inverse");
			pushCMethod(th, quat_mult);
			popProperty(th, 1, "Mult");
		popProperty(th, 0, "_newtype");
		pushCMethod(th, quat_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Quat");
}