#include <cstdlib>
#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "defines.h"
#include "utils.h"

using std::cerr;
using std::endl;


#define POS_ATTRIB      0
#define COLOR_ATTRIB    1

const char vertexShader[] = "#version 150\n"
		"in vec3 vertPos;\n"
		"in vec3 vertColor;\n"
		"out vec3 fragColor;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 proj;\n"

		"void main() {\n"
		"	fragColor = vertColor;\n"
		"	gl_Position = proj * view * model * vec4(vertPos, 1.0);\n"
		"}\n";

const char fragmentShader[] = "#version 150\n"
		"in vec3 fragColor;\n"
		"out vec4 pixelColor;\n"
		"\n"
		"void main() {\n"
		"	pixelColor = vec4(fragColor, 1.0); // RGBA\n"
		"}\n";

static GLfloat vertices[] = {
	// vertex pos       | vertex color
	 0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f
};

int main(int argc, char *argv[])
{
	// initialize GLFW
	if (!glfwInit()) {
		cerr << "Could not initialize GLFW." << endl;
		return EXIT_FAILURE;
	}
	// set some options for glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// create window and its OpenGL context
	GLFWwindow* window = glfwCreateWindow(
				WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
				WINDOW_FULLSCREEN ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	if (window == nullptr) {
		cerr << "Could not create window." << endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	// make the context current
	glfwMakeContextCurrent(window);

	// link OpenGL library
	glewExperimental = GL_TRUE;
	GLenum ret = glewInit();
	if (ret != GLEW_OK) {
		cerr << "Could not initialize GLEW: "
			 << glewGetErrorString(ret) << endl;
		glfwTerminate(); // cleanup GLFW
		return EXIT_FAILURE;
	}

	// initialize shaders
	GLuint program;
	try {
		// create shader objects
		GLuint vert = createShader(vertexShader, "Vertex Shader", GL_VERTEX_SHADER);
		GLuint frag = createShader(fragmentShader, "Fragment Shader", GL_FRAGMENT_SHADER);
		// create program object and attach shaders
		program = createProgram({vert, frag});
		glDeleteShader(vert); glDeleteShader(frag); // we do not need them anymore
		// pre-link specifications
		glBindAttribLocation(program, POS_ATTRIB, "vertPos");
		glBindAttribLocation(program, COLOR_ATTRIB, "vertColor");
		glBindFragDataLocation(program, 0, "pixelColor");
		// link program object
		linkProgram(program);
		validateProgram(program);
	} catch (...) {
		// TODO cleanup shader and program objects
		glfwTerminate();
		return EXIT_FAILURE;
	}

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
				6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(COLOR_ATTRIB);
	glVertexAttribPointer(COLOR_ATTRIB, 3, GL_FLOAT, GL_FALSE,
				6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

	// get uniform locations
	GLint modelUniLoc = glGetUniformLocation(program, "model");
	GLint viewUniLoc = glGetUniformLocation(program, "view");

	// transformations
	glm::mat4 model;
	glm::mat4 view;

	// time of last update
	double lastUpdate = glfwGetTime();
	// cursor position at the last update
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);

	// set projection matrix
	glUseProgram(program);
	float aspect = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
	glm::mat4 proj = glm::perspective(0.8f, aspect, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"),
				1, GL_FALSE, glm::value_ptr(proj));

	// repeat this loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		// clear the color buffer
		glClear(GL_COLOR_BUFFER_BIT);

		// render something with OpenGL
		glUseProgram(program); // select shaders
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

	// free resources from OpenGL
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(program);
	// exit program
	glfwTerminate();
	return EXIT_SUCCESS;
}