/** Manage the 3D display window
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"

#include <stdio.h>

/** C properties describing the display */
struct DisplayInfo {
	SDL_Window *mainWindow;		//!< 3D display window, via SDL
	SDL_GLContext mainContext;	//!< OpenGL context, via SDL
	bool fullscreen;			//!< Is display fullscreen (vs. windowed)?
} *di;

// function prototypes
bool initOpenGL();
void handleSDLError(int line);
void printSDL_GL_Attributes();

/** Initialize SDL, main window, OpenGL, and GLEW */
bool display_newOpenGLWindow(DisplayInfo *di)
{
	// Initialize SDL's Video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Failed to initialize SDL2\n");
		return false;
	}

	// Create centered, resizable window
	di->fullscreen = false;
	SDL_Rect display_rect;
	SDL_GetDisplayBounds(0, &display_rect);
	di->mainWindow = SDL_CreateWindow(PEG_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		display_rect.w*3/4, display_rect.h*3/4, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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

/** Create a new display instance using SDL2 & OpenGL within a window */
int display_new(Value th) {
	pushMember(th, 0, "newtype"); // get the mixin type for instance
	Value dispinst = pushCData(th, popValue(th), sizeof(struct DisplayInfo));
	di = (struct DisplayInfo*) toStr(dispinst);
	display_newOpenGLWindow(di);
	return 1;
}

/** Close down OpenGL, window and SDL2 */
int display_finalizer(Value cdata) {
	struct DisplayInfo *di = (struct DisplayInfo *)(toStr(cdata));

	// Delete our OpengL context
	SDL_GL_DeleteContext(di->mainContext);

	// Destroy our window
	SDL_DestroyWindow(di->mainWindow);

	// Shutdown SDL 2
	SDL_Quit();
	return 1;
}

/** Return aTrue if fullscreen, aFalse if windowed */
int display_getfullscreen(Value th) {
	DisplayInfo *di = (struct DisplayInfo*) toStr(getLocal(th, 0));
	pushValue(th, di->fullscreen? aTrue : aFalse);
	return 1;
}

/** If true, set to fullscreen, otherwise windowed */
int display_setfullscreen(Value th) {
	DisplayInfo *di = (struct DisplayInfo*) toStr(getLocal(th, 0));
	di->fullscreen = !(getTop(th)<2 || isFalse(getLocal(th, 1)));
	SDL_SetWindowFullscreen(di->mainWindow, di->fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	return 1;
}

/** Render $.scene from the point of view of $.camera */
int display_render(Value th) {
	DisplayInfo *di = (struct DisplayInfo*) toStr(getLocal(th, 0));

	// Create render context
	int contexti = getTop(th);
	Value contextv = pushType(th, aNull, 16);

	// Put viewHeight and viewWidth into context
	SDL_Rect display_rect;
	SDL_GetDisplayBounds(0, &display_rect);
	pushValue(th, anInt(display_rect.h));
	popMember(th, contexti, "viewHeight");
	pushValue(th, anInt(display_rect.w));
	popMember(th, contexti, "viewWidth");

	// Get $.camera and $.scene
	pushGloVar(th, "$");
	Value camera = pushMember(th, getTop(th)-1, "camera");
	Value scene = pushMember(th, getTop(th)-2, "scene");
	setTop(th, getTop(th)-3);

	// $.camera.render(context) - adds attributes to context
	pushSym(th, "_render");
	pushValue(th, camera);
	pushValue(th, contextv);
	methodCall(th, 2, 0);

	// $.scene.render(context)
	pushSym(th, "_render");
	pushValue(th, scene);
	pushValue(th, contextv);
	methodCall(th, 2, 0);

	// Swap buffers to display finished image
	SDL_GL_SwapWindow(di->mainWindow);
	return 1;
}

/** Initialize the Display type and '$display'*/
void display_init(Value th) {
	Value Display = pushType(th, aNull, 6);
		pushMixin(th, aNull, aNull, 5);
			pushCMethod(th, display_finalizer);
			popMember(th, 1, "_finalizer");
			pushCMethod(th, display_getfullscreen);
			pushCMethod(th, display_setfullscreen);
			pushClosure(th, 2);
			popMember(th, 1, "fullscreen");
			pushCMethod(th, display_render);
			popMember(th, 1, "_render");
		popMember(th, 0, "newtype");
		pushCMethod(th, display_new);
		popMember(th, 0, "new");
	popGloVar(th, "Display");

	// $display = +Display
	pushSym(th, "new");
	pushValue(th, Display);
	methodCall(th, 1, 1);
	getFromTop(th, 0);
	popGloVar(th, "$display");
}