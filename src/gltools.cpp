#include "gltools.h"

#include <cassert>
#include <cstring>
#include <regex>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef __GNUG__
#include <cxxabi.h>
#include <execinfo.h>
#include <unistd.h>
#endif

#define UTL_LOGGER GL
#include <utl/logging.h>

using std::cmatch;
using std::regex;
using std::string;
using std::stringstream;
using utl::log::Logger;
using utl::log::LogLevel;


bool GLTools::existMissingExtensions = false;
std::unordered_set<std::string> GLTools::supportedExtensions;

bool GLTools::isAvailable(const char *name)
{
	return supportedExtensions.count(name);
}

bool GLTools::isExtensionMissing()
{
	return existMissingExtensions;
}

void GLTools::checkExtension(const char *name, bool required)
{
	bool supported = (glfwExtensionSupported(name) == GL_TRUE);

	const char *s;
	if (supported) {
		s = "available";
		supportedExtensions.emplace(name);
	} else if (required) {
		s = "missing";
		existMissingExtensions = true;
	} else {
		s = "not available";
	}

	assert(std::strncmp(name, "GL_", 3) == 0);
	utl::logl(LogLevel::CONFIG, "%-27s : %s", name + 3, s);
}

void GLTools::registerErrorHandler()
{
	if (isAvailable(GL_KHR_DEBUG)) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(&errorHandler, nullptr);
	} else if (isAvailable(GL_ARB_DEBUG_OUTPUT)) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		// I checked that any output of ARB is supported by KHR.
		// The macros have same values, too.
		glDebugMessageCallbackARB(&errorHandler, nullptr);
	} else {
		utl::warning("Debug output not available");
	}
}

void GLTools::errorHandler(GLenum source, GLenum type, GLuint id,
			GLenum severity, GLsizei length, const GLchar *message,
			const GLvoid *userParam)
{
	LogLevel level = LogLevel::SEVERE;
	string suffix;
	const char *lname, *ename;

	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		level = LogLevel::INFO;
		break;
	case GL_DEBUG_SEVERITY_LOW:
	case GL_DEBUG_SEVERITY_MEDIUM:
		level = LogLevel::WARNING;
		break;
	case GL_DEBUG_SEVERITY_HIGH:
	default:
		level = LogLevel::SEVERE;
		break;
	}

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		lname = "GL.API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		lname = "GL.WINDOW";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		lname = "GL.SHADER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		lname = "GL.THIRD";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		lname = "GL.APP";
		break;
	case GL_DEBUG_SOURCE_OTHER:
	default:
		lname = "GL.OTHER";
		break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		ename = "error: ";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		ename = "deprecated behavior: ";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		ename = "undefined behavior: ";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		ename = "portability issue: ";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		ename = "performance issue: ";
		break;
	case GL_DEBUG_TYPE_MARKER:
	case GL_DEBUG_TYPE_PUSH_GROUP:
	case GL_DEBUG_TYPE_POP_GROUP:
	case GL_DEBUG_TYPE_OTHER:
	default:
		ename = "";
		break;
	}

#ifdef __GNUG__
	if (type == GL_DEBUG_TYPE_ERROR && severity == GL_DEBUG_SEVERITY_HIGH) {
		static void *buffer[256];
		static std::mutex mutex;

		std::lock_guard<std::mutex> lock(mutex);

		void **trace = buffer;
		size_t size = backtrace(trace, 256);

		do {
			trace++;
			size--;
		} while (false); // TODO until left space of OpenGL?

		char **symbols = backtrace_symbols(trace, size);
		if (symbols == nullptr) {
			utl_warning("Could not get symbols. Write direct to stderr:");
			backtrace_symbols_fd(trace, size, STDERR_FILENO);
		} else {
			stringstream sbuf;
			for (size_t i = 0; i < size; ++i) {
				static const regex rgx("([^(]*)\\(([^+]*)\\+([^)]*)\\)\\s*\\[(.*)\\]");
				cmatch m;
				if (regex_match(symbols[i], m, rgx)) {
					string bin = m[1].str(), ref = m[2].str(),
							offset = m[3].str(), addr = m[4].str();
					// TODO use addr2line or something like this to get line in sources

					int status = -1;
					char *fn = abi::__cxa_demangle(ref.c_str(), nullptr, nullptr, &status);
					if (status == 0) {
						ref = fn;
						free(fn);
					} else if (status != -2) {
						utl_warning("Could no demangle %s", ref.c_str());
					}
					sbuf << "\nin " << (ref.empty() ? "??" : ref) << " +"
						 << offset << " (" << bin << ")" << " [" << addr << "]";
				} else {
					sbuf << "\n" << symbols[i];
				}
			}
			suffix = sbuf.str();
			free(symbols);
		}
	}
#endif

	// remove newlines from message
	string msg = message;
	{
		std::size_t i = msg.find_last_not_of("\r\n");
		msg = msg.substr(0, (i == string::npos ? 0 : i));
	}

	Logger &logger = Logger::get(lname);
	logger.log(level, "%s%s%s", ename, msg.c_str(), suffix.c_str());
}
