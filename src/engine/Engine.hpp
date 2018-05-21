//#pragma once
//
//
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <imgui.h>
//#include <imgui_impl_glfw_gl3.h>
//
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <map>
//#include <vector>
//#include <tuple>
//#include <cstdio>
//#include <cstdlib>
//#include <iostream>
//#include <random>
//#include <ctime>
//
//#include "../model/model.hpp"
//#include "../scene/Scene.hpp"
//#include "../gui/Gui.hpp"
//#include "../camera/camera.hpp"
//#include "../processing/shader.hpp"
//#include "../processing/texture.hpp"
//#include "../processing/shape.hpp"
//#include "../processing/light.hpp"
//
//
//// TODO Add exception handling (throw, try, catch)
//// TODO Move methods definitions to .cpp file!
//class Engine {
//private:
//	GLFWwindow* window;
//	Gui gui;
//	// TODO Declare arrays for storing objects that should be rendered in the scene
//	// Any model, texture, shader loading put into a separate class which appends its objects 
//	// to aforementioned array/vector
//public:
//	static const GLuint WIDTH = 640;
//	static const GLuint HEIGHT = 480;
//	static const unsigned int OPENGL_MAJOR_VERSION = 4;
//	static const unsigned int OPENGL_MINOR_VERSION = 0;
//	static const bool RESIZABLE_WINDOW = GL_TRUE;
//
//	Engine();
//	Engine(const Engine&) = delete;
//	~Engine();
//	
//	void InitCallbacks();
//	void InitWindow();
//	void Run();
//
//	static void loadShaders();
//};