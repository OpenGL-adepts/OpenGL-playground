#pragma once

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
#include <ctime>

// TODO Add exception handling (throw, try, catch)
// TODO Move methods definitions to .cpp file!

class Engine {
private:
	GLFWwindow* window;
	// TODO Declare arrays for storing objects that should be rendered in the scene
	// Any model, texture, shader loading put into a separate class which appends its objects 
	// to aforementioned array/vector
public:
	static const GLuint WIDTH = 640;
	static const GLuint HEIGHT = 480;
	static const unsigned int OPENGL_MAJOR_VERSION = 4;
	static const unsigned int OPENGL_MINOR_VERSION = 0;
	static const bool RESIZABLE_WINDOW = GL_TRUE;

	////////////
	// Callbacks
	////////////
	static void ErrorCallback(int error, const char* description)
	{
		std::cerr << "Error: " << description << std::endl;
	}

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
			std::cout << "Example - Key [E] pressed" << std::endl;
		else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			std::cout << "Exiting..." << std::endl;
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		// Display in place
		std::cout << '\r' << "Mouse: (" << xpos << ", " << ypos << ")";
	}

	///////////////
	// Init methods
	///////////////
	void InitCallbacks() {
		glfwSetErrorCallback(ErrorCallback);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetCursorPosCallback(window, CursorPositionCallback);
	}

	void InitWindow() {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, RESIZABLE_WINDOW);

		#if __APPLE__
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		#endif


		// Useful for full-screen mode etc.
		GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* glfwMode = glfwGetVideoMode(glfwMonitor);

		// TODO Extract window name to static const
		window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL-playground", nullptr, nullptr);
		if (window == nullptr)
		{
			glfwTerminate();
			exit(EXIT_FAILURE);
		}
		glfwMakeContextCurrent(window);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		gladLoadGL();
		glViewport(0, 0, WIDTH, HEIGHT);
	}

	void InitRandomNumbersGenerator() {
		std::srand(std::time(0));
	}

	void InitGUI() {

	}

	Engine() {
		if (glfwInit() == GLFW_TRUE) {
			InitWindow();
			InitCallbacks();
			InitGUI();
			InitRandomNumbersGenerator();
		}
		else {
			exit(EXIT_FAILURE);
		}
	}

	~Engine() {
		// TODO Double check if no memory leaks occur
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void UpdateGUI() {
		// TODO Put ImGui code here
	}

	void UpdateScene(float& color) {
		color += 0.0001;
		glClearColor(0.0, color, 0.3, 1.0);
	}

	void Run() {
		// TODO Delete in the future
		float color = 0.0;

		while (!glfwWindowShouldClose(window))
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Rendering examples
			UpdateGUI();
			UpdateScene(color);
			
			// Finally apply screen updates
			glfwSwapBuffers(window);

			// Handle input/window movement
			glfwPollEvents();
		}
	}
};