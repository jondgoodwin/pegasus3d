/** Define the World type and its global instance '$'
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include "xyzmath.h"

/** Create a new world */
int world_new(Value th) {
	pushType(th, getLocal(th, 0), 32); // Create prototype of self (World)
	return 1;
}

/** Get running flag */
int world_getrunning(Value th) {
	pushCloVar(th, 2);
	return 1;
}

/** Get running flag */
int world_setrunning(Value th) {
	pushLocal(th, 1);
	popCloVar(th, 2);
	return 1;
}

void world_stoprunning(Value th) {
	pushSym(th, "running?");
	pushLocal(th, 0);
	pushValue(th, aFalse);
	setCall(th, 2, 0);
}

/** Do a full frame: handleInput, send ticks and render
 * dt is passed as the parameter. */
int world_nextframe(Value th) {
	// Default dt, just in case
	if (getTop(th)<2)
		pushValue(th, aFloat(0.013333f));

	// Process all queued input
	pushSym(th, "handleInput");
	pushValue(th, getLocal(th, 0));
	getCall(th, 1, 0);

	// Update the state each tick
	pushSym(th, "updateState");
	pushValue(th, getLocal(th,0));
	pushValue(th, getLocal(th,1));
	getCall(th, 2, 0);

	// $window.render (uses $.camera and $.scene)
	pushSym(th, "_Render");
	pushValue(th, getLocal(th, 0));
	getCall(th, 1, 0);
	return 1;
}

float camYRotation = 0.0f;
float camVelocity = 0.0f;
float camYRotRate = 0.0f;

/** Default method for updating the world's state every tick.
    dt is first parameter (delta time since last update in seconds). */
int world_update(Value th) {
	// Get dt
	GLfloat dt = 0.01333f;
	if (getTop(th)>1 && isFloat(getLocal(th, 1)))
		dt = toAfloat(getLocal(th, 1));

	// Move the camera slowly up
	pushGloVar(th, "$");
	int camidx = getTop(th);
	pushProperty(th, getTop(th)-1, "camera");
	Value posv = pushProperty(th, camidx, "location");
	if (isCData(posv)) {
		Xyz *posxyz = (Xyz *)toHeader(posv);
		// posxyz->y += dt;
		camYRotation+=camYRotRate;
		if (camYRotation<0.0)
			camYRotation+=(float)M_PI * 2.0f;
		if (camYRotation>2*M_PI)
			camYRotation-=(float)M_PI * 2.0f;
		posxyz->x += camVelocity * sin(camYRotation);
		posxyz->z += -camVelocity * cos(camYRotation);
	}
	Value rotv = pushProperty(th, camidx, "rotation");
	if (isCData(rotv)) {
		Xyz *rotxyz = (Xyz *)toHeader(rotv);
		rotxyz->y = camYRotation;
	}
	setTop(th, 1);
	return 1;
}

/** Check for any input, and then handle appropriately */
int world_handleInput(Value th)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// Quit event
		if (event.type == SDL_QUIT)
			world_stoprunning(th);

		// Window resize
	    if (event.type == SDL_WINDOWEVENT) {
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				glViewport(0, 0, event.window.data1, event.window.data2);
				break;
			}
		}

		// Keyboard events
		else if (event.type == SDL_KEYUP)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:
			case SDLK_DOWN:
				camVelocity = 0;
				break;
			case SDLK_LEFT:
			case SDLK_RIGHT:
				camYRotRate = 0;
				break;
			}
		}

		// Keyboard events
		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				world_stoprunning(th);
				break;

			// Toggle fullscreen mode for world's window
			case SDLK_F11:
				{
				pushSym(th, "fullscreen");
				Value window = pushProperty(th, 0, "window");
				pushSym(th, "fullscreen");
				pushValue(th, window);
				getCall(th, 1, 1);
				{Value val = getFromTop(th, 0);}
				pushValue(th, isFalse(popValue(th))? aTrue : aFalse);
				setCall(th, 2, 0);
				} break;

			case SDLK_UP:
				camVelocity = 0.08f;
				break;
			case SDLK_DOWN:
				camVelocity = -0.08f;
				break;
			case SDLK_LEFT:
				camYRotRate = -0.08f;
				break;
			case SDLK_RIGHT:
				camYRotRate = 0.08f;
				break;

			default:
				break;
			}
		}
	}
	return 1;
}

/** Render the world's scene via .camera to .window */
int world_render(Value th) {
	int selfidx = 0;

	// Set up a render of the world's window, creating it if needed
	pushSym(th, "_Render");
	if (pushProperty(th, selfidx, "window")==aNull) {
		popValue(th);
		pushSym(th, "New");
		pushGloVar(th, "Window");
		getCall(th, 1, 1);
		pushValue(th, getFromTop(th, 0));
		popProperty(th, selfidx, "window");
	}

	// Create render context
	int contextidx = getTop(th);
	pushSym(th, "New");
	pushGloVar(th, "RenderContext");
	getCall(th, 1, 1);

	// Store scene in the context
	pushProperty(th, selfidx, "scene");
	popProperty(th, contextidx, "scene");

	// Store camera in the context, creating if needed using default view
	if (pushProperty(th, selfidx, "camera")==aNull) {
		popValue(th);
		pushSym(th, "New");
		pushGloVar(th, "Camera");
		getCall(th, 1, 1);
		pushValue(th, getFromTop(th, 0));
		popProperty(th, selfidx, "camera");
	}
	popProperty(th, contextidx, "camera");

	// Render the window, passing context
	getCall(th, 2, 0);
	return 0;
}

/** Initialize the World type */
void world_init(Value th) {
	pushType(th, aNull, 6);
		pushSym(th, "World");
		popProperty(th, 0, "_name");
		pushCMethod(th, world_new);
		popProperty(th, 0, "New");
		pushCMethod(th, world_getrunning);
		pushCMethod(th, world_setrunning);
		pushValue(th, aTrue);
		pushClosure(th, 3);
		popProperty(th, 0, "running?");
		pushCMethod(th, world_nextframe);
		popProperty(th, 0, "nextFrame");
		pushCMethod(th, world_handleInput);
		popProperty(th, 0, "handleInput");
		pushCMethod(th, world_update);
		popProperty(th, 0, "updateState");
		pushCMethod(th, world_render);
		popProperty(th, 0, "_Render");
	popGloVar(th, "World");
}