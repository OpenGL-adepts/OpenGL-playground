#include "Engine.hpp"

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

Engine::Engine() {
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
Engine::~Engine() {
	// TODO Double check if no memory leaks occur
	glfwDestroyWindow(window);
	glfwTerminate();
}

///////////////
// Init methods
///////////////
void Engine::InitCallbacks() {
	glfwSetErrorCallback(ErrorCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, CursorPositionCallback);
}

void Engine::InitWindow() {
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
	
	// Setup Dear ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window, true);

	glfwSwapInterval(1); // Enable vsync*/

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	gladLoadGL();
	glViewport(0, 0, WIDTH, HEIGHT);
}

void Engine::InitRandomNumbersGenerator() {
	std::srand(std::time(0));
}

void Engine::InitGUI() {
	// Delegate responsibility to gui.cpp implementation
}

void Engine::UpdateGUI() {
	// Delegate responsibility to gui.cpp implementation
}


void Engine::UpdateScene(float& color) {
	color += 0.0001;
	glClearColor(0.0, color, 0.3, 1.0);
}

void Engine::Run() {
	// TODO Delete in the future
	float color = 0.0;

	Model model;
	model.loadModel("C:/Users/karols/Desktop/OpenGL-playground/resources/objects/sheep.obj");


	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		// Rendering examples
		UpdateGUI();
		UpdateScene(color);

		model.Draw();

		ImGui::Text("Hello, word!");	
		ImGui::Render();

		// Finally apply screen updates
		glfwSwapBuffers(window);

		// Handle input/window movement
		glfwPollEvents();
	}
	
	// Cleanup
	glfwDestroyWindow(window);
	glfwTerminate();
}

////////////
// Callbacks
////////////
void Engine::ErrorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

void Engine::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	// Display in place
	std::cout << '\r' << "Mouse: (" << xpos << ", " << ypos << ")";
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		std::cout << "Example - Key [E] pressed" << std::endl;
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		std::cout << "Exiting..." << std::endl;
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}