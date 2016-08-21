/** Manage the 3D display window
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

#include <stdio.h>

/** C properties describing the window */
struct WindowInfo {
	SDL_Window *sdlWindow;		//!< 3D display window, via SDL
	SDL_GLContext sdlContext;	//!< OpenGL context, via SDL
	bool fullscreen;			//!< Are we fullscreen (vs. windowed)?
} *di;

/** Print out the received SDL error */
void logSDLError(const char *message)
{
	const char* error = SDL_GetError();
	if (error != "") {
		vmLog("%s. SDL error message: %s", message, error);
		SDL_ClearError();
	}
}

/** Initialize SDL, main window, OpenGL, and GLEW */
bool window_newOpenGLWindow(WindowInfo *di)
{
	// Create centered, resizable window
	di->fullscreen = false;
	SDL_Rect window_rect;
	SDL_GetDisplayBounds(0, &window_rect);
	di->sdlWindow = SDL_CreateWindow(PEG_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_rect.w*3/4, window_rect.h*3/4, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!di->sdlWindow) {
		logSDLError("Unable to open window");
		return false;
	}

	// Create opengl context and attach it to the window
	di->sdlContext = SDL_GL_CreateContext(di->sdlWindow);

	// Initialize OpenGL attributes
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
	// SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	// SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	// Turn on double buffering with a 24-bit Z buffer.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    /* Enable multisampling for an antialiased effect */
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Synchronize buffer swap with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	return true;
}

/** Create a new window instance using SDL2 & OpenGL within a window */
int window_new(Value th) {
	pushProperty(th, 0, "newtype"); // get the mixin type for instance
	Value dispinst = strHasFinalizer(pushCData(th, popValue(th), PegWindow, 0, sizeof(struct WindowInfo)));
	di = (struct WindowInfo*) toHeader(dispinst);
	window_newOpenGLWindow(di);
	return 1;
}

/** Close down OpenGL, window and SDL2 */
int window_finalizer(Value cdata) {
	struct WindowInfo *di = (struct WindowInfo *)(toHeader(cdata));

	// Delete our OpengL context
	SDL_GL_DeleteContext(di->sdlContext);

	// Destroy our window
	SDL_DestroyWindow(di->sdlWindow);

	return 1;
}

/** Return aTrue if fullscreen, aFalse if windowed */
int window_getfullscreen(Value th) {
	WindowInfo *di = (struct WindowInfo*) toHeader(getLocal(th, 0));
	pushValue(th, di->fullscreen? aTrue : aFalse);
	return 1;
}

/** If true, set to fullscreen, otherwise windowed */
int window_setfullscreen(Value th) {
	WindowInfo *di = (struct WindowInfo*) toHeader(getLocal(th, 0));
	di->fullscreen = !(getTop(th)<2 || isFalse(getLocal(th, 1)));
	SDL_SetWindowFullscreen(di->sdlWindow, di->fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	return 1;
}

/** Render scene from the point of view of camera, context as first parameter */
int window_render(Value th) {

	// Switch OpenGL to work within this window and clear the buffers
	WindowInfo *di = (struct WindowInfo*) toHeader(getLocal(th, 0));
	SDL_GL_MakeCurrent(di->sdlWindow, di->sdlContext);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Closer objects obscure further objects

	// The first parameter is the context
	int contextidx = 1;
	int i = getTop(th);
	Value c = getLocal(th, contextidx);

	// Initialize OpenGL rendering style
	glUseProgram(0); // No shader

	// Put viewHeight and viewWidth into context
	SDL_Rect window_rect;
	SDL_GetDisplayBounds(0, &window_rect);
	pushValue(th, anInt(window_rect.h));
	popProperty(th, contextidx, "viewHeight");
	pushValue(th, anInt(window_rect.w));
	popProperty(th, contextidx, "viewWidth");

	// context.camera._Render(context) - adds attributes to context
	pushSym(th, "_RenderIt");
	pushProperty(th, contextidx, "camera"); // camera
	pushLocal(th, contextidx);
	getCall(th, 2, 0);

	// context.scene._Render(context)
	pushSym(th, "_Render");
	pushProperty(th, contextidx, "scene");
	pushLocal(th, contextidx);
	getCall(th, 2, 0);

	// Swap buffers to display window's finished image
	SDL_GL_SwapWindow(di->sdlWindow);
	return 0;
}

/** Initialize the Window type and '$window'*/
void window_init(Value th) {
	pushType(th, aNull, 6);
		pushMixin(th, aNull, aNull, 5);
			pushCMethod(th, window_finalizer);
			popProperty(th, 1, "_finalizer");
			pushCMethod(th, window_getfullscreen);
			pushCMethod(th, window_setfullscreen);
			pushClosure(th, 2);
			popProperty(th, 1, "fullscreen");
			pushCMethod(th, window_render);
			popProperty(th, 1, "_Render");
		popProperty(th, 0, "newtype");
		pushCMethod(th, window_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Window");
}