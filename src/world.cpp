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

	pushSym(th, "animate");
	pushLocal(th, 0);
	pushLocal(th, 1);
	getCall(th, 2, 0);

	setTop(th, 1);
	return 1;
}

void pushKeySym(Value th, SDL_Scancode key) {
	switch(key) {
	case SDL_SCANCODE_0: pushSym(th, "key_0"); break;
	case SDL_SCANCODE_1: pushSym(th, "key_1"); break;
	case SDL_SCANCODE_2: pushSym(th, "key_2"); break;
	case SDL_SCANCODE_3: pushSym(th, "key_3"); break;
	case SDL_SCANCODE_4: pushSym(th, "key_4"); break;
	case SDL_SCANCODE_5: pushSym(th, "key_5"); break;
	case SDL_SCANCODE_6: pushSym(th, "key_6"); break;
	case SDL_SCANCODE_7: pushSym(th, "key_7"); break;
	case SDL_SCANCODE_8: pushSym(th, "key_8"); break;
	case SDL_SCANCODE_9: pushSym(th, "key_9"); break;
	case SDL_SCANCODE_A: pushSym(th, "key_A"); break;
	case SDL_SCANCODE_B: pushSym(th, "key_B"); break;
	case SDL_SCANCODE_C: pushSym(th, "key_C"); break;
	case SDL_SCANCODE_D: pushSym(th, "key_D"); break;
	case SDL_SCANCODE_E: pushSym(th, "key_E"); break;
	case SDL_SCANCODE_F: pushSym(th, "key_F"); break;
	case SDL_SCANCODE_G: pushSym(th, "key_G"); break;
	case SDL_SCANCODE_H: pushSym(th, "key_H"); break;
	case SDL_SCANCODE_I: pushSym(th, "key_I"); break;
	case SDL_SCANCODE_J: pushSym(th, "key_J"); break;
	case SDL_SCANCODE_K: pushSym(th, "key_K"); break;
	case SDL_SCANCODE_L: pushSym(th, "key_L"); break;
	case SDL_SCANCODE_M: pushSym(th, "key_M"); break;
	case SDL_SCANCODE_N: pushSym(th, "key_N"); break;
	case SDL_SCANCODE_O: pushSym(th, "key_O"); break;
	case SDL_SCANCODE_P: pushSym(th, "key_P"); break;
	case SDL_SCANCODE_Q: pushSym(th, "key_Q"); break;
	case SDL_SCANCODE_R: pushSym(th, "key_R"); break;
	case SDL_SCANCODE_S: pushSym(th, "key_S"); break;
	case SDL_SCANCODE_T: pushSym(th, "key_T"); break;
	case SDL_SCANCODE_U: pushSym(th, "key_U"); break;
	case SDL_SCANCODE_V: pushSym(th, "key_V"); break;
	case SDL_SCANCODE_W: pushSym(th, "key_W"); break;
	case SDL_SCANCODE_X: pushSym(th, "key_X"); break;
	case SDL_SCANCODE_Y: pushSym(th, "key_Y"); break;
	case SDL_SCANCODE_Z: pushSym(th, "key_Z"); break;

	case SDL_SCANCODE_F1: pushSym(th, "key_f1"); break;
	case SDL_SCANCODE_F2: pushSym(th, "key_f2"); break;
	case SDL_SCANCODE_F3: pushSym(th, "key_f3"); break;
	case SDL_SCANCODE_F4: pushSym(th, "key_f4"); break;
	case SDL_SCANCODE_F5: pushSym(th, "key_f5"); break;
	case SDL_SCANCODE_F6: pushSym(th, "key_f6"); break;
	case SDL_SCANCODE_F7: pushSym(th, "key_f7"); break;
	case SDL_SCANCODE_F8: pushSym(th, "key_f8"); break;
	case SDL_SCANCODE_F9: pushSym(th, "key_f9"); break;
	case SDL_SCANCODE_F10: pushSym(th, "key_f10"); break;
	case SDL_SCANCODE_F11: pushSym(th, "key_f11"); break;
	case SDL_SCANCODE_F12: pushSym(th, "key_f12"); break;

	case SDL_SCANCODE_APOSTROPHE: pushSym(th, "key_apostrophe"); break;
	case SDL_SCANCODE_BACKSLASH: pushSym(th, "key_backslash"); break;
	case SDL_SCANCODE_BACKSPACE: pushSym(th, "key_backspace"); break;
	case SDL_SCANCODE_CAPSLOCK: pushSym(th, "key_capslock"); break;
	case SDL_SCANCODE_COMMA: pushSym(th, "key_comma"); break;
	case SDL_SCANCODE_DELETE: pushSym(th, "key_delete"); break;
	case SDL_SCANCODE_DOWN: pushSym(th, "key_down"); break;
	case SDL_SCANCODE_EQUALS: pushSym(th, "key_equals"); break;
	case SDL_SCANCODE_ESCAPE: pushSym(th, "key_escape"); break;
	case SDL_SCANCODE_GRAVE: pushSym(th, "key_grave"); break;
	case SDL_SCANCODE_END: pushSym(th, "key_end"); break;
	case SDL_SCANCODE_HOME: pushSym(th, "key_home"); break;
	case SDL_SCANCODE_INSERT: pushSym(th, "key_insert"); break;
	case SDL_SCANCODE_LALT: pushSym(th, "key_lalt"); break;
	case SDL_SCANCODE_LCTRL: pushSym(th, "key_lctrl"); break;
	case SDL_SCANCODE_LEFT: pushSym(th, "key_left"); break;
	case SDL_SCANCODE_LSHIFT: pushSym(th, "key_lshift"); break;
	case SDL_SCANCODE_MINUS: pushSym(th, "key_minus"); break;
	case SDL_SCANCODE_PAGEDOWN: pushSym(th, "key_pagedown"); break;
	case SDL_SCANCODE_PAGEUP: pushSym(th, "key_pageup"); break;
	case SDL_SCANCODE_PAUSE: pushSym(th, "key_pause"); break;
	case SDL_SCANCODE_PERIOD: pushSym(th, "key_period"); break;
	case SDL_SCANCODE_PRINTSCREEN: pushSym(th, "key_printscreen"); break;
	case SDL_SCANCODE_RALT: pushSym(th, "key_ralt"); break;
	case SDL_SCANCODE_RCTRL: pushSym(th, "key_rctrl"); break;
	case SDL_SCANCODE_RETURN: pushSym(th, "key_return"); break;
	case SDL_SCANCODE_RIGHT: pushSym(th, "key_right"); break;
	case SDL_SCANCODE_RIGHTBRACKET: pushSym(th, "key_rightbracket"); break;
	case SDL_SCANCODE_RSHIFT: pushSym(th, "key_rshift"); break;
	case SDL_SCANCODE_SEMICOLON: pushSym(th, "key_semicolon"); break;
	case SDL_SCANCODE_SLASH: pushSym(th, "key_slash"); break;
	case SDL_SCANCODE_SPACE: pushSym(th, "key_space"); break;
	case SDL_SCANCODE_SYSREQ: pushSym(th, "key_sysreq"); break;
	case SDL_SCANCODE_TAB: pushSym(th, "key_tab"); break;
	case SDL_SCANCODE_UP: pushSym(th, "key_up"); break;
	default: pushValue(th, aNull);
	}
}

/** Check for any input, and then handle appropriately */
int world_handleInput(Value th)
{
	int inputIdx = getTop(th);
	Value inputContext = pushProperty(th, 0, "input");
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
			default:
				if (inputContext!=aNull && event.key.repeat==0) {
					Value keydown = pushProperty(th, inputIdx, "keyUp");
					pushKeySym(th, event.key.keysym.scancode);
					pushValue(th, getProperty(th, keydown, getFromTop(th, 0)));
					pushValue(th, aNull);
					getCall(th, 1, 1);
					popValue(th);
					popValue(th);
				}
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
				if (inputContext!=aNull && event.key.repeat==0) {
					Value keydown = pushProperty(th, inputIdx, "keyDown");
					pushKeySym(th, event.key.keysym.scancode);
					pushValue(th, getProperty(th, keydown, getFromTop(th, 0)));
					pushValue(th, aNull);
					getCall(th, 1, 1);
					popValue(th);
					popValue(th);
				}
				break;
			}
		}
	}
	popValue(th);
	return 0;
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