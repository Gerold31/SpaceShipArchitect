#ifndef GLTOOLS_H
#define GLTOOLS_H

#include <string>
#include <unordered_set>

#include <GL/glew.h>
#ifdef _WIN32
#  include <windows.h>
#else
#  define APIENTRY
#endif


class GLTools
{
public:
	GLTools() = delete;

	static constexpr const char *GL_KHR_DEBUG = "GL_KHR_debug";
	static constexpr const char *GL_ARB_DEBUG_OUTPUT = "GL_ARB_debug_output";

	static bool isAvailable(const char *name);
	static bool isExtensionMissing();
	static void checkExtension(const char *name, bool required);

	static void registerErrorHandler();

	static void APIENTRY errorHandler(GLenum source, GLenum type, GLuint id,
				GLenum severity, GLsizei length, const GLchar *message,
				const GLvoid *userParam);

private:
	static bool existMissingExtensions;
	static std::unordered_set<std::string> supportedExtensions;
};

#endif // GLTOOLS_H
