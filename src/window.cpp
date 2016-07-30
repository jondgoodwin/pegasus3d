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
	SDL_Window *mainWindow;		//!< 3D display window, via SDL
	SDL_GLContext mainContext;	//!< OpenGL context, via SDL
	bool fullscreen;			//!< Are we fullscreen (vs. windowed)?
} *di;

// function prototypes
bool initOpenGL();
void handleSDLError(int line);
void printSDL_GL_Attributes();

/** Initialize SDL, main window, OpenGL, and GLEW */
bool window_newOpenGLWindow(WindowInfo *di)
{
	// Initialize SDL's Video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Failed to initialize SDL2\n");
		return false;
	}

	// Create centered, resizable window
	di->fullscreen = false;
	SDL_Rect window_rect;
	SDL_GetDisplayBounds(0, &window_rect);
	di->mainWindow = SDL_CreateWindow(PEG_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_rect.w*3/4, window_rect.h*3/4, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// Check that everything worked out okay
	if (!di->mainWindow)
	{
		printf("Unable to create main window\n");
		handleSDLError(__LINE__);
		return false;
	}

	// Create our opengl context and attach it to our window
	di->mainContext = SDL_GL_CreateContext(di->mainWindow);

	// Initialize global OpenGL attributes
	initOpenGL();

	// Synchronize our buffer swap with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);

	// Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	return true;
}

/** Initialize global OpenGL attributes */
bool initOpenGL()
{
	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    /* Enable multisampling for a nice antialiased effect */
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	return true;
}

/** Print out the received SDL error */
void handleSDLError(int line = -1)
{
	const char* error = SDL_GetError();

	if (error != "")
	{
		if (line < 0)
			printf("SDL Error: %s\n", error);
		else
			printf("SDL Error (line %d): %s\n", line, error);

		SDL_ClearError();
	}
}

/** Output the OpenGL version number */
void printSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	printf("SDL_GL_CONTEXT_MAJOR_VERSION: %d\n", value);

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	printf("SDL_GL_CONTEXT_MINOR_VERSION: %d\n", value);
}

/** Create a new window instance using SDL2 & OpenGL within a window */
int window_new(Value th) {
	pushProperty(th, 0, "newtype"); // get the mixin type for instance
	Value dispinst = pushCData(th, popValue(th), 0, sizeof(struct WindowInfo));
	di = (struct WindowInfo*) toHeader(dispinst);
	window_newOpenGLWindow(di);
	return 1;
}

/** Close down OpenGL, window and SDL2 */
int window_finalizer(Value cdata) {
	struct WindowInfo *di = (struct WindowInfo *)(toHeader(cdata));

	// Delete our OpengL context
	SDL_GL_DeleteContext(di->mainContext);

	// Destroy our window
	SDL_DestroyWindow(di->mainWindow);

	// Shutdown SDL 2
	SDL_Quit();
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
	SDL_SetWindowFullscreen(di->mainWindow, di->fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	return 1;
}

/** Render scene (2nd parm) from the point of view of camera (1st parm) */
int window_render(Value th) {
	if (getTop(th)<3)
		return 1;

	WindowInfo *di = (struct WindowInfo*) toHeader(getLocal(th, 0));

	// Create render context
	int contexti = getTop(th);
	Value contextv = pushType(th, aNull, 16);

	// Put viewHeight and viewWidth into context
	SDL_Rect window_rect;
	SDL_GetDisplayBounds(0, &window_rect);
	pushValue(th, anInt(window_rect.h));
	popProperty(th, contexti, "viewHeight");
	pushValue(th, anInt(window_rect.w));
	popProperty(th, contexti, "viewWidth");

	// $.camera.render(context) - adds attributes to context
	pushSym(th, "render");
	pushValue(th, getLocal(th, 1)); // camera
	pushValue(th, contextv);
	getCall(th, 2, 0);

	// $.scene.render(context)
	pushSym(th, "render");
	pushValue(th, getLocal(th, 2)); // scene
	pushValue(th, contextv);
	getCall(th, 2, 0);

	// Swap buffers to window's finished image
	SDL_GL_SwapWindow(di->mainWindow);
	return 1;
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
			popProperty(th, 1, "render");
		popProperty(th, 0, "newtype");
		pushCMethod(th, window_new);
		popProperty(th, 0, "new");
	popGloVar(th, "Window");
}