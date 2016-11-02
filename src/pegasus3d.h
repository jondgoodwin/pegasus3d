/** Include file for Pegasus3d.
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * Copyright (C) 2016  Jonathan Goodwin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef pegasus3d_h
#define pegasus3d_h

#define PEG_VERSION_MAJOR	"0"		//!< Major version number
#define PEG_VERSION_MINOR	"1"		//!< Minor version number
#define PEG_VERSION_NUM		0		//!< Version number
#define PEG_VERSION_RELEASE	"4"		//!< Version release

#define PEG_NAME "Pegasus3d Browser"
#define PEG_VERSION	PEG_NAME " " PEG_VERSION_MAJOR "." PEG_VERSION_MINOR		//!< Full version string
#define PEG_RELEASE	PEG_VERSION "." PEG_VERSION_RELEASE		//!< Full version+release string
#define PEG_COPYRIGHT	PEG_RELEASE "  Copyright (C) 2016 Jonathan Goodwin"		//!< Name + version + copyright string

// Acorn VM library
#define AVM_LIBRARY_STATIC
#include <avm.h>
#ifdef __cplusplus
using namespace avm;
#endif

// GLEW Headers - for OpenGL
#define GL3_PROTOTYPES 1
#include <GL/glew.h>

// SDL2 Headers
#include <SDL.h>

enum PegCDataTypes {
	Vec2Value,
	XyzValue,
	QuatValue,
	ColorValue,
	Mat2Value,
	Mat3Value,
	Mat4Value,

	RectValue,
	ArrayValue,
	ShaderValue,
	WindowValue,
	ImageValue,

	// Only needed in Array
	FloatNbr,
	Uint8Nbr,
	Uint16Nbr,
	Uint32Nbr
};

/** Structure for an Xyz value */
typedef struct Xyz {
	GLfloat x;	//!< x
	GLfloat y;	//!< y
	GLfloat z;	//!< z
} Xyz;

#define toXyz(value) ((Xyz*) toHeader(value)) //<! Point to value's Xyz data
#define isXyz(value) (isCDataType(value, XyzValue)) //<! Does value contain Xyz data

/** Structure for a Quat(ernion) value */
typedef struct Quat {
	GLfloat x;	//!< x
	GLfloat y;	//!< y
	GLfloat z;	//!< z
	GLfloat w;  //!< w
} Quat;

#define toQuat(value) ((Quat*) toHeader(value)) //<! Point to value's Quat data
#define isQuat(value) (isCDataType(value, QuatValue)) //<! Does value contain Quat data

/** Structure for a Color value */
struct ColorInfo {
	float red;		//!< red component (0-1)
	float green;	//!< green component (0-1)
	float blue;		//!< blue component (0-1)
	float alpha;	//!< alpha component (0=invisible, 1=opaque)
};

#define toColor(value) ((ColorInfo*) toHeader(value)) //<! Point to value's Color data
#define isColor(value) (isCDataType(value, ColorValue)) //<! Does value contain Color data

/** Structure for a 4x4 matrix */
typedef GLfloat Mat4[16];
#define toMat4(value) ((Mat4*) toHeader(value)) //<! Point to value's Mat4 data
#define isMat4(value) (isCDataType(value, Mat4Value)) //<! Does value contain Mat4 data

/** Structure for a 2d image rectangle */
typedef struct Rect {
	int x;  //!< x position
	int y;  //!< y position
	int w;  //!< width
	int h;	//!< height
} Rect;

#define toRect(value) ((Rect*) toHeader(value)) //<! Point to value's Rect data
#define isRect(value) (isCDataType(value, RectValue)) //<! Does value contain Rect data

/** Structure for the header for a collection of number structures */
struct ArrayHeader {
	AuintIdx nStructs;	//!< number of structures in the array
	char mbrType;		//!< float/int and number of bytes in a number
	char structSz;		//!< How many numbers in a structure
};

#define toArrayHeader(value) ((ArrayHeader*) toHeader(value)) //<! Point to value's ArrayHeader data

/** Structure for an Image value's header */
struct ImageHeader {
	AuintIdx x;
	AuintIdx y;
	AuintIdx z;
	unsigned char nbytes;
};

#define toImageHeader(value) ((ImageHeader*) toHeader(value)) //<! Point to value's ImageHeader data

#endif