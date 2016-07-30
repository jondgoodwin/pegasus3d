/** Shader compilation and use
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include <stdio.h>
#include <stdlib.h>

#include "pegasus3d.h"
#include "xyzmath.h"

/** Structure for holding a ready-to-use shader program.
  We do this so that we can depend on a finalizer to delete
  a program when it is no longer referenced. */
struct ShaderPgm {
	GLuint program;	//!< Handle for OpenGL shader program
};

/** Compile a shader program */
GLuint shader_compile(const char *shadersource, GLenum shadertype) {
	int IsCompiled;
	int maxLength;
    char *InfoLog;

    /* This is the handle used to reference the shader */
    GLuint shader;

    /* Create an empty vertex shader handle */
    shader = glCreateShader(shadertype);

    /* Send the vertex shader source code to GL */
    /* Note that the source code is NULL character terminated. */
    /* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
    glShaderSource(shader, 1, (const GLchar**)&shadersource, 0);

    /* Compile the vertex shader */
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &IsCompiled);
    if(IsCompiled == GL_FALSE)
    {
       glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

       /* The maxLength includes the NULL character */
       InfoLog = (char *)malloc(maxLength);

       glGetShaderInfoLog(shader, maxLength, &maxLength, InfoLog);

       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(InfoLog);
       return 0;
    }
	return shader;
}

/** Compile and bind shaders into a shader program */
Value shader_make(Value th, Value pgmv) {
	GLuint vshader;
	GLuint fshader;
    int IsLinked;
    int maxLength;
    char *shaderProgramInfoLog;

	Value vertex = pushProperty(th, 0, "vertex"); popValue(th);
	vshader = shader_compile(toStr(vertex), GL_VERTEX_SHADER);
	Value fragment = pushProperty(th, 0, "fragment"); popValue(th);
	fshader = shader_compile(toStr(fragment), GL_FRAGMENT_SHADER);
	if (vshader==0 || fshader==0)
		return aNull;

    /* If we reached this point it means the vertex and fragment shaders compiled and are syntax error free. */
    /* We must link them together to make a GL shader program */
    /* GL shader programs are monolithic. It is a single piece made of 1 vertex shader and 1 fragment shader. */
    /* Assign our program handle a "name" */
    GLuint shaderprogram = glCreateProgram();

    /* Attach our shaders to our program */
    glAttachShader(shaderprogram, vshader);
    glAttachShader(shaderprogram, fshader);

    /* Bind "in" attributes in listed order. Binding must happen before a link. */
	Value inlist = pushProperty(th, 0, "in"); popValue(th);
	if (isArr(inlist)) {
		for (AuintIdx i=0; i < getSize(inlist); i++) {
			glBindAttribLocation(shaderprogram, i, toStr(arrGet(th, inlist, i)));
		}
	}

    /* Link our program */
    /* At this stage, the vertex and fragment programs are inspected, optimized and a binary code is generated for the shader. */
    /* The binary code is uploaded to the GPU, if there is no error. */
    glLinkProgram(shaderprogram);
    glDetachShader(shaderprogram, vshader);
    glDetachShader(shaderprogram, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    /* Again, we must check and make sure that it linked. If it fails, it would mean either there is a mismatch between the vertex */
    /* and fragment shaders. It might be that you have surpassed your GPU's abilities. Perhaps too many ALU operations or */
    /* too many texel fetch instructions or too many interpolators or dynamic loops. */

    glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
    if (IsLinked == GL_FALSE)
    {
       /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
       glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

       /* The maxLength includes the NULL character */
       shaderProgramInfoLog = (char *)malloc(maxLength);

       /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
       glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);

       /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
       /* In this simple program, we'll just leave */
       free(shaderProgramInfoLog);
       return aNull;
    }

	// Remember compiled program, then return as success
	ShaderPgm* p = (ShaderPgm*) toHeader(pgmv);
	p->program = shaderprogram;
	return pgmv;
}

/** Close out a shader that is no longer referenced anywhere */
int shader_closepgm(Value shaderpgm) {
	ShaderPgm *pgm = (ShaderPgm*) toHeader(shaderpgm);
	glDeleteProgram(pgm->program);
	return 1;
}

/** Create a new shader */
int shader_new(Value th) {
	pushType(th, getLocal(th, 0), 16); // Create prototype of self (Shader)
	return 1;
}

/** Render the shader */
int shader_render(Value th) {
	// Get compiled shader, if it exists
	Value pgmv = pushProperty(th, 0, "_program");
	if (pgmv==aNull) {
		// If it does not exist, compile and bind it based on info
		Value pgmtype = pushProperty(th, 0, "_compiledtype");
		pgmv = pushCData(th, pgmtype, 0, sizeof(ShaderPgm)); // Is small enough to stick in header
		if (aNull != (pgmv = shader_make(th, pgmv)))
			popProperty(th, 0, "_program");
		else
			popValue(th);
	}

    /* Load the shader into the rendering pipeline */
	if (pgmv != aNull) {
		ShaderPgm *pgmdata = (ShaderPgm*) toHeader(pgmv);
		glUseProgram(pgmdata->program);

		// Load all the shader's named "uniform" values from the context
		Value uniformlist = pushProperty(th, 0, "uniform"); popValue(th);
		if (isArr(uniformlist)) {
			for (AuintIdx i=0; i < getSize(uniformlist); i++) {
				Value uninamev = arrGet(th, uniformlist, i);
				Value unival = getProperty(th, getLocal(th, 1), uninamev);
				switch (sizeof(Mat4) /*getSize(unival)*/) { // Hack!! Fix it!!
				case sizeof(Mat4):
					glUniformMatrix4fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, GL_FALSE, (GLfloat *) toStr(unival));
					break;
				default:
					assert(false && "Unsupported uniform type!!!");
				}
			}
		}

		// Put the shader's "in" list on the context as "vertexAttributes"
		pushLocal(th, 1); popValue(th);
		pushProperty(th, 0, "in");
		popProperty(th, 1, "vertexAttributes");
	}
	return 1;
}

/** Initialize shader type */
void shader_init(Value th) {
	Value Shader = pushType(th, aNull, 16);
		pushCMethod(th, shader_new);
		popProperty(th, 0, "new");
		pushCMethod(th, shader_render);
		popProperty(th, 0, "render");
		Value pgmmmixin = pushMixin(th, aNull, aNull, 4);
			pushCMethod(th, shader_closepgm);
			popProperty(th, 1, "_finalizer");
		popProperty(th, 0, "_compiledtype");
	popGloVar(th, "Shader");
}