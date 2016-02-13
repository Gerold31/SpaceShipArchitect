#include "utils.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <vector>

using std::cerr;
using std::endl;
using std::exception;
using std::vector;


GLuint loadShader(const std::string &filename, GLenum type)
{
	// load sources for shader
	std::ifstream stream(filename);
	std::string source((std::istreambuf_iterator<char>(stream)),
					   (std::istreambuf_iterator<char>()));
	// create and return shader
	return createShader(source, filename, type);
}

GLuint createShader(const std::string &source, const std::string &name, GLenum type)
{
	const char *sources[] = {source.c_str()};
	// create a new shader
	GLuint shader = glCreateShader(type);
	assert(shader != 0);
	// set sources for shader
	glShaderSource(shader, 1, sources, nullptr);
	// compile shader
	glCompileShader(shader);
	// check if the shader was compiled correctly
	GLint isCompiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled != GL_TRUE) {
		// something went wrong while compiling shader
		cerr << "Could not compile shader: " << name << endl;
		// get log for compilation and write it to stderr
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		vector<GLchar> buffer(length);
		glGetShaderInfoLog(shader, length, &length, buffer.data());
		cerr << buffer.data() << endl;
		// throw exception
		throw exception();
	}
	// return shader
	return shader;
}

GLuint createProgram(const std::vector<GLuint> &shaders)
{
	// create a new shader program
	GLuint program = glCreateProgram();
	assert(program != 0);
	// add shaders to shader program
	for (GLuint shader : shaders) {
		glAttachShader(program, shader);
	}
	// return shader program
	return program;
}

void linkProgram(GLuint program)
{
	// link shader program
	glLinkProgram(program);
	// check if the program was linked correctly
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked != GL_TRUE) {
		// something went wrong while linking program
		cerr << "Could not link program." << endl;
		// get log for linking and write it to stderr
		GLint length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		vector<GLchar> buffer(length);
		glGetProgramInfoLog(program, length, &length, buffer.data());
		cerr << buffer.data() << endl;
		// throw exception
		throw exception();
	}
}

void validateProgram(GLuint program)
{
	glValidateProgram(program);
	// get validation status
	GLint isValid = 0;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &isValid);
	if (isValid != GL_TRUE)
		cerr << "Program is not valid" << endl;
	// write log to stderr if available
	GLint length;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	if (length != 0) {
		vector<GLchar> buffer(length);
		glGetProgramInfoLog(program, length, &length, buffer.data());
		cerr << buffer.data() << endl;
	}
	// throw exception if not valid
	if (isValid != GL_TRUE)
		throw exception();
}
