#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#include <GL/glew.h>


GLuint loadShader(const std::string &filename, GLenum type);
GLuint createShader(const std::string &source, const std::string &name, GLenum type);
GLuint createProgram(const std::vector<GLuint> &shaders);
void linkProgram(GLuint program);
void validateProgram(GLuint program);

#endif // UTILS_H
