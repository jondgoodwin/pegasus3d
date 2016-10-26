/** Manage the 3D display window, including main window ($window)
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
};

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
	pushProperty(th, 0, "_newtype"); // get the mixin type for instance
	Value dispinst = strHasFinalizer(pushCData(th, popValue(th), PegWindow, 0, sizeof(struct WindowInfo)));
	WindowInfo *wininfo = (struct WindowInfo*) toHeader(dispinst);
	window_newOpenGLWindow(wininfo);
	return 1;
}

/** Close down OpenGL, window and SDL2 */
int window_finalizer(Value cdata) {
	struct WindowInfo *wininfo = (struct WindowInfo *)(toHeader(cdata));

	// Delete our OpengL context
	SDL_GL_DeleteContext(wininfo->sdlContext);

	// Destroy our window
	SDL_DestroyWindow(wininfo->sdlWindow);

	return 0;
}

/** Return aTrue if fullscreen, aFalse if windowed */
int window_getfullscreen(Value th) {
	WindowInfo *wininfo = (struct WindowInfo*) toHeader(getLocal(th, 0));
	pushValue(th, wininfo->fullscreen? aTrue : aFalse);
	return 1;
}

/** If true, set to fullscreen, otherwise windowed */
int window_setfullscreen(Value th) {
	WindowInfo *wininfo = (struct WindowInfo*) toHeader(getLocal(th, 0));
	wininfo->fullscreen = !(getTop(th)<2 || isFalse(getLocal(th, 1)));
	SDL_SetWindowFullscreen(wininfo->sdlWindow, wininfo->fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	return 0;
}

/** Attach current OpenGL context to this window */
int window_makecurrent(Value th) {
	WindowInfo *wininfo = (struct WindowInfo*) toHeader(getLocal(th, 0));
	SDL_GL_MakeCurrent(wininfo->sdlWindow, wininfo->sdlContext);
	if (getTop(th)>1 && isCDataType(getLocal(th, 1), PegRect)) {
		Rect *winrect = (Rect*) toHeader(getLocal(th,1));
		winrect->x = winrect->y = 0;
		SDL_GetWindowSize(wininfo->sdlWindow, &winrect->w, &winrect->h);
	}
	return 0;
}

/** Swap window's buffers, displaying what we have rendered. */
int window_swapbuffers(Value th) {
	WindowInfo *wininfo = (struct WindowInfo*) toHeader(getLocal(th, 0));
	SDL_GL_SwapWindow(wininfo->sdlWindow);
	return 0;
}

/** Browser's main window */
struct WindowInfo mainWindow;
/** Create main window */
void window_newMainWindow(void) {
	window_newOpenGLWindow(&mainWindow);
}
/** Destroy main window */
void window_destroyMainWindow(void) {
	SDL_GL_DeleteContext(mainWindow.sdlContext);
	SDL_DestroyWindow(mainWindow.sdlWindow);
}

/** Initialize the Window type and '$window'*/
void window_init(Value th) {
	pushType(th, aNull, 6);
		pushSym(th, "Window");
		popProperty(th, 0, "_name");
		Value newtype = pushMixin(th, aNull, aNull, 5);
			pushSym(th, "*Window");
			popProperty(th, 1, "_name");
			pushCMethod(th, window_finalizer);
			popProperty(th, 1, "_finalizer");
			pushCMethod(th, window_getfullscreen);
			pushCMethod(th, window_setfullscreen);
			pushClosure(th, 2);
			popProperty(th, 1, "fullscreen");
			pushCMethod(th, window_makecurrent);
			popProperty(th, 1, "MakeCurrent");
			pushCMethod(th, window_swapbuffers);
			popProperty(th, 1, "SwapBuffers");
		popProperty(th, 0, "_newtype");
		pushCMethod(th, window_new);
		popProperty(th, 0, "New");
	popGloVar(th, "Window");

	// Create $window for main window
	pushValue(th, newtype); // get the mixin type for a Window
	Value windowv = pushCData(th, popValue(th), PegWindow, 0, sizeof(struct WindowInfo));
	WindowInfo *wininfo = (struct WindowInfo*) toHeader(windowv);
	wininfo->sdlWindow = mainWindow.sdlWindow;
	wininfo->sdlContext = mainWindow.sdlContext;
	wininfo->fullscreen = mainWindow.fullscreen;
	popGloVar(th, "$window");
}