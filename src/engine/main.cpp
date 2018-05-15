// GLEngine by Joshua Senouf - 2016
// Credits to Joey de Vries (LearnOpenGL) and Kevin Fung (Glitter)



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <tuple>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>

GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

int main(int argc, char* argv[])
{
	std::cout << "@micmarty" << std::endl;

	// Basic GLFW Setup
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* glfwMode = glfwGetVideoMode(glfwMonitor);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL-sandbox", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	gladLoadGL();
	glViewport(0, 0, WIDTH, HEIGHT);

	// Engine loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Drawing etc goes here

		// Check for errors
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << error << std::endl;
			break;
		}

		// Finally swap buffers
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
