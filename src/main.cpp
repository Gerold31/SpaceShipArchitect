#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gtl/ogl/program.h>
#include <gtl/ogl/texture.h>

#define UTL_LOGGER main
#include <utl/log/consoleloghandler.h>
#include <utl/logging.h>

#include "config.h"
#include "defines.h"
#include "gltools.h"
#include "resourceloader.h"
#include "utils.h"

using utl::log::ConsoleLogHandler;
using utl::log::Logger;


#define POS_ATTRIB      0
#define COLOR_ATTRIB    1
#define TEXCORD_ATTRIB  2

static GLfloat vertices[] = {
	// vertex pos       | vertex color      | tex cord
	 0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
	 0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f
};

int main(int argc, char *argv[])
{
	// Initialize loggers
	Logger::getRoot().addHandler(std::make_shared<ConsoleLogHandler>());

	// initialize GLFW
	if (!glfwInit()) {
		utl::severe("Could not initialize GLFW.");
		return EXIT_FAILURE;
	}
	// set some options for glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// create window and its OpenGL context
	GLFWwindow* window = glfwCreateWindow(
				WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
				WINDOW_FULLSCREEN ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	if (window == nullptr) {
		utl::severe("Could not create window.");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	// make the context current
	glfwMakeContextCurrent(window);

	// link OpenGL library
	glewExperimental = GL_TRUE;
	GLenum ret = glewInit();
	if (ret != GLEW_OK) {
		utl::severe("Could not initialize GLEW: %s", glewGetErrorString(ret));
		glfwTerminate(); // cleanup GLFW
		return EXIT_FAILURE;
	}

	// check and initialize OpenGL context
	GLTools::checkExtension(GLTools::GL_KHR_DEBUG, false);
	GLTools::checkExtension(GLTools::GL_ARB_DEBUG_OUTPUT, false);
	GLTools::checkExtension("GL_ARB_direct_state_access", true);
	GLTools::checkExtension("GL_ARB_separate_shader_objects", true);

	if (GLTools::isExtensionMissing()) {
		utl::severe("Required OpenGL extensions are missing.");
		glfwTerminate(); // cleanup GLFW
		return EXIT_FAILURE;
	}
	GLTools::registerErrorHandler();

	utl::info("Load and initialize resources ...");
	// create resource loader
	ResourceLoader resources(RESOURCE_DIR);

	// initialize shaders
	gtl::ogl::Program program = resources.loadShaderProgram("shader/example.prog");

	// load textures
	gtl::ogl::Texture texture = resources.loadTexture("texture/test_rect.png");

	// initialize vertex buffer object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// initialize vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(POS_ATTRIB);
	glVertexAttribPointer(POS_ATTRIB, 3, GL_FLOAT, GL_FALSE,
				8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(COLOR_ATTRIB);
	glVertexAttribPointer(COLOR_ATTRIB, 3, GL_FLOAT, GL_FALSE,
				8 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(TEXCORD_ATTRIB);
	glVertexAttribPointer(TEXCORD_ATTRIB, 2, GL_FLOAT, GL_FALSE,
				8 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat)));

	// get uniform locations
	GLint modelUniLoc = program.getUniformLocation("model");
	GLint viewUniLoc = program.getUniformLocation("view");

	// transformations
	glm::mat4 model;
	glm::mat4 view;

	// time of last update
	double lastUpdate = glfwGetTime();
	// cursor position at the last update
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);

	// set projection matrix
	float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
	glm::mat4 proj = glm::perspective(0.8f, aspect, 0.1f, 1000.0f);
	program.setUniform(program.getUniformLocation("proj"), proj);

	// set bindings for samplers
	program.setUniform(program.getUniformLocation("tex"), 1);

	utl::info("Setup complete.");
	// repeat this loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		// clear the color buffer
		glClear(GL_COLOR_BUFFER_BIT);

		// render something with OpenGL
		program.use(); // select shaders
		texture.bind(1);
		glBindVertexArray(vao);
		glUniformMatrix4fv(modelUniLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewUniLoc, 1, GL_FALSE, glm::value_ptr(view));
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// poll events (I do it before swapping buffers to get more fps)
		glfwPollEvents();

		// compute delta time `dt` (time elapsed since last frame)
		double dt = lastUpdate;
		lastUpdate = glfwGetTime();
		dt = lastUpdate - dt;

		// rotate triangle
		glm::vec3 up = glm::vec3(0.0f, 0.1f, 0.0f);
		model = glm::rotate(model, static_cast<float>(dt), up);

		// move cam
		// --- update cam position
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			movement.z -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			movement.z += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			movement.x -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			movement.x += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			movement.y -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			movement.y += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			movement *= 2;
		}
		view = glm::translate(glm::mat4(), - movement * static_cast<float>(dt)) * view;
		// --- update cam rotation
		glm::vec3 rotation(0.0f, 0.0f, 0.0f);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS
				|| glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
			double newX, newY;
			glfwGetCursorPos(window, &newX, &newY);
			glfwSetCursorPos(window, cursorX, cursorY);
			rotation.x = -0.001 * ( newY - cursorY ); // rotation about x axis
			rotation.y = -0.001 * ( newX - cursorX ); // rotation about y axis
		} else {
			glfwGetCursorPos(window, &cursorX, &cursorY);
		}
		view = glm::rotate(glm::mat4(), - rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) * view;
		view = glm::rotate(glm::mat4(), - rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * view;
		//view = glm::rotate(glm::mat4(), - rotation.y, glm::vec3(0.0f, 0.0f, 1.0f)) * view;

		// swap front and back buffers
		glfwSwapBuffers(window);
	}

	utl::info("Clean up resources ...");
	// free resources from OpenGL
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	// exit program
	glfwTerminate();
	return EXIT_SUCCESS;
}
