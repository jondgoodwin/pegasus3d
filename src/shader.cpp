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

    // This is the handle used to reference the shader
    GLuint shader;

    // Create an empty vertex shader handle
    shader = glCreateShader(shadertype);

    // Send the vertex shader source code to GL 
    // Note that the source code is NULL character terminated.
    // GL will automatically detect that therefore the length info can be 0 in this case (the last parameter)
    glShaderSource(shader, 1, (const GLchar**)&shadersource, 0);

    // Compile the vertex shader and check for errors
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &IsCompiled);
    if(IsCompiled == GL_FALSE)
    {
       glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
       InfoLog = (char *)malloc(maxLength);
       glGetShaderInfoLog(shader, maxLength, &maxLength, InfoLog);
	   vmLog("Shader compilation failure: %s", InfoLog);
       free(InfoLog);
       return 0;
    }
	return shader;
}

/** Compile and bind shaders into a shader program */
Value shader_make(Value th, Value pgmv) {
	int selfidx = 0;
	GLuint vshader;
	GLuint fshader;
    int IsLinked;
    int maxLength;
    char *shaderProgramInfoLog;

	Value vertex = pushProperty(th, selfidx, "vertex"); popValue(th);
	vshader = shader_compile(toStr(vertex), GL_VERTEX_SHADER);
	Value fragment = pushProperty(th, selfidx, "fragment"); popValue(th);
	fshader = shader_compile(toStr(fragment), GL_FRAGMENT_SHADER);
	if (vshader==0 || fshader==0)
		return aNull;

    // Link the shaders together to create a GL shader program
    GLuint shaderprogram = glCreateProgram();
    glAttachShader(shaderprogram, vshader);
    glAttachShader(shaderprogram, fshader);

	// Bind "attributes" in listed order.
	Value inlist = pushProperty(th, selfidx, "attributes");
	if (isArr(inlist)) {
		for (AuintIdx i=0; i < getSize(inlist); i++) {
			glBindAttribLocation(shaderprogram, i, toStr(arrGet(th, inlist, i)));
		}
	}
	popValue(th);

    // Link the program, then upload it to the GPU.
    glLinkProgram(shaderprogram);
    glDetachShader(shaderprogram, vshader);
    glDetachShader(shaderprogram, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    // Check for linkage errors, e.g.: a mismatch between the vertex and fragment shaders.
    // Also possible: surpassed your GPU's abilities, too many ALU operations,
    // too many texel fetch instructions or too many interpolators or dynamic loops.
    glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
    if (IsLinked == GL_FALSE)
    {
       glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);
       shaderProgramInfoLog = (char *)malloc(maxLength);
       glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);
	   vmLog("Shader program linkage error: %s", shaderProgramInfoLog);
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

/** Render the shader, retrieving uniforms from context as parameter 1 */
int shader_render(Value th) {
	int selfidx = 0;
	int contextidx = 1;
	int texunit = 0;

	// Get compiled shader, if it exists
	Value pgmv = pushProperty(th, selfidx, "_program");
	if (pgmv==aNull) {
		// If it does not exist, compile and bind it based on info
		Value pgmtype = pushProperty(th, selfidx, "_compiledtype");
		pgmv = strHasFinalizer(pushCData(th, pgmtype, PegShaderPgm, 0, sizeof(ShaderPgm))); // Is small enough to stick in header
		if (aNull != (pgmv = shader_make(th, pgmv)))
			popProperty(th, selfidx, "_program");
		else
			popValue(th);
	}

	/* Load the shader into the rendering pipeline */
	if (pgmv != aNull) {
		ShaderPgm *pgmdata = (ShaderPgm*) toHeader(pgmv);
		glUseProgram(pgmdata->program);

		// Calculate mvpmatrix = pmatrix * mvmatrix
		pushSym(th, "New");
		pushGloVar(th, "Matrix4");
		getCall(th, 1, 1);
		Mat4 *mvp = (Mat4*) toHeader(getFromTop(th, 0));
		Value pmatrix = pushProperty(th, contextidx, "pmatrix"); popValue(th);
		Value mvmatrix = pushProperty(th, contextidx, "mvmatrix"); popValue(th);
		mat4Mult(mvp, (Mat4*) toHeader(pmatrix), (Mat4*) toHeader(mvmatrix));
		popProperty(th, contextidx, "mvpmatrix");

		// Load all the shader's named "uniform" values from the context
		Value uniformlist = pushProperty(th, selfidx, "uniforms"); popValue(th);
		if (isArr(uniformlist)) {
			for (AuintIdx i=0; i < getSize(uniformlist); i++) {
				Value uninamev = arrGet(th, uniformlist, i);
				Value unival = getProperty(th, getLocal(th, contextidx), uninamev);
				if (isFloat(unival))
					glUniform1f(glGetUniformLocation(pgmdata->program, toStr(uninamev)), toAfloat(unival));
				else if (isInt(unival))
					glUniform1i(glGetUniformLocation(pgmdata->program, toStr(uninamev)), toAint(unival));
				else if (isCData(unival)) {
					switch(getCDataType(unival)) {
					case PegMat2: glUniformMatrix2fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, GL_FALSE, (GLfloat *) toHeader(unival)); break;
					case PegMat3: glUniformMatrix3fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, GL_FALSE, (GLfloat *) toHeader(unival)); break;
					case PegMat4: 
						glUniformMatrix4fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, GL_FALSE, (GLfloat *) toHeader(unival)); break;
					//case PegUint32: glUniform1iv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), univalhdr->nStructs, (GLint *) toCData(unival)); break;
					case PegFloat: glUniform1fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, (GLfloat *) toCData(unival)); break;
					case PegVec2: glUniform2fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, (GLfloat *) toHeader(unival)); break;
					case PegVec3: glUniform3fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, (GLfloat *) toHeader(unival)); break;
					case PegVec4: 
						glUniform4fv(glGetUniformLocation(pgmdata->program, toStr(uninamev)), 1, (GLfloat *) toHeader(unival)); break;
					default: 
						//const char *x = toStr(uninamev);
						assert(false && "Unsupported uniform type!!!");
					}
				} else if (isType(unival)) {
					pushSym(th, "name");
					Value name = getProperty(th, unival, getFromTop(th, 0));
					popValue(th);
					if (isEqStr(name, "Texture")) {
						pushSym(th, "_Render");
						pushValue(th, unival);
						pushLocal(th, contextidx);
						getCall(th, 2, 1);
						glUniform1i(glGetUniformLocation(pgmdata->program, toStr(uninamev)), toAint(popValue(th)));
					}
				}
			}
		}
	}
	else
		glUseProgram(0);

	return 0;
}

/** Initialize shader type */
void shader_init(Value th) {
	Value Shader = pushType(th, aNull, 16);
		pushSym(th, "Shader");
		popProperty(th, 0, "_name");
		pushCMethod(th, shader_new);
		popProperty(th, 0, "New");
		pushCMethod(th, shader_render);
		popProperty(th, 0, "_Render");
		Value pgmmmixin = pushMixin(th, aNull, aNull, 4);
			pushSym(th, "*Shader");
			popProperty(th, 1, "_name");
			pushCMethod(th, shader_closepgm);
			popProperty(th, 1, "_finalizer");
		popProperty(th, 0, "_compiledtype");
	popGloVar(th, "Shader");
}