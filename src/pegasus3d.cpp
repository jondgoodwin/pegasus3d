/** Pegasus3d Browser - main program
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

#include <stdio.h>
#include <string.h>

/** Handle for Acorn VM context */
Value vm;

void color_init(Value th);
void xyz_init(Value th);
void mat4_init(Value th);
void display_init(Value th);
void world_init(Value th);
void camera_init(Value th);
void scene_init(Value th);
void mesh_init(Value th);
void shader_init(Value th);

void test_init(Value th);

/** Initialize World Types and environment global variables. */
void initTypes(Value th) {
	// Pure data types
	color_init(th);
	xyz_init(th);
	mat4_init(th);

	// World types - the order to initialize matters
	display_init(th);
	world_init(th);
	camera_init(th);
	scene_init(th);
	mesh_init(th);
	shader_init(th);
}

// Initialize, run main loop, close up shop
int main(int argc, char *argv[])
{
	freopen("pegasus3d.log", "w", stderr);

	// Start Acorn VM and load its types
	Value th = newVM();
	initTypes(th);
	
	test_init(th);	// temporary hack

	// Get current world from "$" (and its running state)
	Value curworld = pushGloVar(th, "$"); popValue(th);
	pushSym(th, "running?");
	pushValue(th, curworld);
	methodCall(th, 1, 1);
	Value isrunning = popValue(th);

	Uint32 thisTime = SDL_GetTicks();
	Uint32 lastTime;

	// Do the event loop forever, until someone stops it
	while (!isFalse(isrunning))
	{
		// Process all queued input
		pushSym(th, "handleInput");
		pushValue(th, curworld);
		methodCall(th, 1, 0);

		// Update the state each tick
		lastTime = thisTime;
		thisTime = SDL_GetTicks();
		pushSym(th, "updateState");
		pushValue(th, curworld);
		pushValue(th, aFloat(((Afloat)(thisTime-lastTime))/1000.0f));
		methodCall(th, 2, 0);

		// $display.render (uses $.camera and $.scene)
		pushSym(th, "_render");
		pushGloVar(th, "$display");
		methodCall(th, 1, 0);

		// Refresh getting current world and its running state
		curworld = pushGloVar(th, "$"); popValue(th);
		pushSym(th, "running?");
		pushValue(th, curworld);
		methodCall(th, 1, 1);
		isrunning = popValue(th);
	}

	vm_close(th);

	return 0;
}

