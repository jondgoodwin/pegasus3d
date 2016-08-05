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
#define PEG_VERSION_MINOR	"0"		//!< Minor version number
#define PEG_VERSION_NUM		0		//!< Version number
#define PEG_VERSION_RELEASE	"2"		//!< Version release

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

#endif