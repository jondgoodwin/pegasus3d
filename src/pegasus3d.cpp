/** Pegasus3d Browser - main program
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "curl/curl.h"

#include <stdio.h>
#include <string.h>

/** Handle for Acorn VM context */
Value vm;

void color_init(Value th);
void xyz_init(Value th);
void mat4_init(Value th);
void window_init(Value th);
void world_init(Value th);
void camera_init(Value th);
void scene_init(Value th);
void shape_init(Value th);
void shader_init(Value th);

void http_init(Value th);

void test_init(Value th);

/** Initialize World Types and environment global variables. */
void initTypes(Value th) {
	// Pure data types
	color_init(th);
	xyz_init(th);
	mat4_init(th);

	// World types
	world_init(th);
	window_init(th);
	camera_init(th);
	scene_init(th);
	shape_init(th);
	shader_init(th);

	http_init(th);
}

/** Initialize $ to be a blank world */
void initWorld(Value th) {
	// $ = +World
	int curworld = getTop(th);
	pushSym(th, "new");
	pushGloVar(th, "World");
	getCall(th, 1, 1);

	// $.scene = +Scene
	pushSym(th, "new");
	pushGloVar(th, "Scene");
	getCall(th, 1, 1);
	popProperty(th, curworld, "scene");

	// ... and store it
	popGloVar(th, "$");
}

// Initialize, run main loop, close up shop
int main(int argc, char *argv[])
{
	freopen("pegasus3d.log", "w", stderr);

	// Initialize SDL's Video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		vmLog("Failed to initialize SDL2's Video subsystem\n");
		return 1;
	}

	// Start Acorn VM and load its types
	Value th = newVM();
	initTypes(th);

	// Initialize $
	initWorld(th);
	test_init(th);	// temporary hack for building a test world

	// Load and run the world whose url was passed as first parameter	
	curl_global_init(CURL_GLOBAL_WIN32);
	pushSym(th, "()");
	pushGloVar(th, "Resource");
#ifdef _DEBUG
	pushString(th, aNull, argc>1? argv[1] : "file://./test.acn");
#else
	pushString(th, aNull, argc>1? argv[1] : "http://ddd.jondgoodwin.com/world.acn");
#endif
	getCall(th, 2, 0);

	// Initialize timer count and running state
	Value isrunning = aTrue;
	Uint32 thisTime = SDL_GetTicks();
	Uint32 lastTime;

	// Do the event loop forever, until someone stops it
	while (!isFalse(isrunning))
	{
		// Do next frame (passing dt)
		pushSym(th, "nextFrame");
		pushGloVar(th, "$");
		lastTime = thisTime;
		thisTime = SDL_GetTicks();
		pushValue(th, aFloat(((Afloat)(thisTime-lastTime))/1000.0f));
		getCall(th, 2, 0);

		// Refresh getting current world and its running state
		pushSym(th, "running?");
		pushGloVar(th, "$");
		getCall(th, 1, 1);
		isrunning = popValue(th);
	}

	curl_global_cleanup(); // Shutdown curl
	vmClose(th); // Shutdown Acorn VM
	SDL_Quit(); // Shutdown SDL2

	return 0;
}

