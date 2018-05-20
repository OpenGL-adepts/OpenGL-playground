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

Shader gBufferShader;
Shader latlongToCubeShader;
Shader simpleShader;
Shader lightingBRDFShader;
Shader irradianceIBLShader;
Shader prefilterIBLShader;
Shader integrateIBLShader;
Shader firstpassPPShader;

int WIDTH = 640;
int HEIGHT = 480;

Texture envMapHDR;
Texture envMapCube;
Texture envMapIrradiance;
Texture envMapPrefilter;
Texture envMapLUT;

Light lightPoint1;
Light lightPoint2;
Light lightPoint3;
Light lightDirectional1;

Shape quadRender;
Shape envCubeRender;

glm::vec3 lightPointPosition1 = glm::vec3(1.5f, 0.75f, 1.0f);
glm::vec3 lightPointPosition2 = glm::vec3(-1.5f, 1.0f, 1.0f);
glm::vec3 lightPointPosition3 = glm::vec3(0.0f, 0.75f, -1.2f);
glm::vec3 lightPointColor1 = glm::vec3(1.0f);
glm::vec3 lightPointColor2 = glm::vec3(1.0f);
glm::vec3 lightPointColor3 = glm::vec3(1.0f);
glm::vec3 lightDirectionalDirection1 = glm::vec3(-0.2f, -1.0f, -0.3f);
glm::vec3 lightDirectionalColor1 = glm::vec3(1.0f);

GLfloat lightPointRadius1 = 3.0f;
GLfloat lightPointRadius2 = 3.0f;
GLfloat lightPointRadius3 = 3.0f;

GLuint gBuffer, zBuffer, gPosition, gNormal, gAlbedo, gEffects;
GLuint postprocessFBO, postprocessBuffer;

GLuint envToCubeFBO, irradianceFBO, prefilterFBO, brdfLUTFBO, envToCubeRBO, irradianceRBO, prefilterRBO, brdfLUTRBO;

glm::mat4 projViewModel;
glm::mat4 prevProjViewModel = projViewModel;
glm::mat4 envMapProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 envMapView[] =
{
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
glm::vec3 modelPosition = glm::vec3(0.0f);
glm::vec3 modelRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 modelScale = glm::vec3(0.1f);
GLfloat modelRotationSpeed = 0.0f;
GLint gBufferView = 1;
bool iblMode = true;

void gBufferSetup();
void postprocessSetup();
void iblSetup();


Engine::Engine() {
	if (glfwInit() == GLFW_TRUE) {
		InitWindow();
		InitCallbacks();
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
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, ScrollCallback);
}

void Engine::InitWindow() {

	glfwInit();

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

	ImGui_ImplGlfwGL3_Init(window, true);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	gladLoadGL();
	glViewport(0, 0, WIDTH, HEIGHT);
}

void Engine::Run() {

	//----------
	// Shader(s)
	//----------
#define PROJ_ROOT "C:/stuff/MSI/OpenGL-playground/"

	gBufferShader.setShader(PROJ_ROOT "resources/shaders/gBuffer.vert", PROJ_ROOT "resources/shaders/gBuffer.frag");
	latlongToCubeShader.setShader(PROJ_ROOT "resources/shaders/latlongToCube.vert", PROJ_ROOT "resources/shaders/latlongToCube.frag");

	simpleShader.setShader(PROJ_ROOT "resources/shaders/lighting/simple.vert", PROJ_ROOT "resources/shaders/lighting/simple.frag");

	lightingBRDFShader.setShader( PROJ_ROOT "resources/shaders/lighting/lightingBRDF.vert",   PROJ_ROOT "resources/shaders/lighting/lightingBRDF.frag");
	irradianceIBLShader.setShader(PROJ_ROOT "resources/shaders/lighting/irradianceIBL.vert",  PROJ_ROOT "resources/shaders/lighting/irradianceIBL.frag");
	prefilterIBLShader.setShader( PROJ_ROOT "resources/shaders/lighting/prefilterIBL.vert",   PROJ_ROOT "resources/shaders/lighting/prefilterIBL.frag");
	integrateIBLShader.setShader( PROJ_ROOT "resources/shaders/lighting/integrateIBL.vert",   PROJ_ROOT "resources/shaders/lighting/integrateIBL.frag");
	firstpassPPShader.setShader(  PROJ_ROOT "resources/shaders/postprocess/postprocess.vert", PROJ_ROOT "resources/shaders/postprocess/firstpass.frag");


	//-----------
	// Textures(s)
	//-----------

	envMapHDR.setTextureHDR(PROJ_ROOT "resources/textures/hdr/popcorn.hdr", "space", true);

	envMapCube.setTextureCube(512, GL_RGB, GL_RGB16F, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR);
	envMapIrradiance.setTextureCube(32, GL_RGB, GL_RGB16F, GL_FLOAT, GL_LINEAR);
	envMapPrefilter.setTextureCube(128, GL_RGB, GL_RGB16F, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR);
	envMapPrefilter.computeTexMipmap();
	envMapLUT.setTextureHDR(512, 512, GL_RG, GL_RG16F, GL_FLOAT, GL_LINEAR);

	// TODO: create a button in imGui to load the scene
	// TODO: create a button in imGui to save the scene
	Model m;
	//Scene scene;
	//scene.Load(PROJ_ROOT "resources/scene.json");
	//scene.Save(PROJ_ROOT "sceneExperiment.json");
	m.loadModel(PROJ_ROOT "resources/models/sphere/sphere.obj");


	//---------------
	// Shape(s)
	//---------------
	envCubeRender.setShape("cube", glm::vec3(0.0f));
	quadRender.setShape("quad", glm::vec3(0.0f));


	//----------------
	// Light source(s)
	//----------------
	lightPoint1.setLight(lightPointPosition1, glm::vec4(lightPointColor1, 1.0f), lightPointRadius1, true);
	lightPoint2.setLight(lightPointPosition2, glm::vec4(lightPointColor2, 1.0f), lightPointRadius2, true);
	lightPoint3.setLight(lightPointPosition3, glm::vec4(lightPointColor3, 1.0f), lightPointRadius3, true);

	lightDirectional1.setLight(lightDirectionalDirection1, glm::vec4(lightDirectionalColor1, 1.0f));


	//---------------------------------------------------------
	// Set the samplers for the lighting/post-processing passes
	//---------------------------------------------------------
	lightingBRDFShader.useShader();
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "gAlbedo"), 1);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "gNormal"), 2);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "gEffects"), 3);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "envMap"), 5);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "envMapIrradiance"), 6);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "envMapPrefilter"), 7);
	glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "envMapLUT"), 8);

	firstpassPPShader.useShader();
	glUniform1i(glGetUniformLocation(firstpassPPShader.Program, "gEffects"), 2);

	latlongToCubeShader.useShader();
	glUniform1i(glGetUniformLocation(latlongToCubeShader.Program, "envMap"), 0);

	irradianceIBLShader.useShader();
	glUniform1i(glGetUniformLocation(irradianceIBLShader.Program, "envMap"), 0);

	prefilterIBLShader.useShader();
	glUniform1i(glGetUniformLocation(prefilterIBLShader.Program, "envMap"), 0);


	//---------------
	// G-Buffer setup
	//---------------
	gBufferSetup();


	//---------------------
	// Postprocessing setup
	//---------------------
	postprocessSetup();


	//----------
	// IBL setup
	//----------
	iblSetup();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		gui->initGui(window);

		//------------------------
		// Geometry Pass rendering
		//------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		// Camera setting
		glm::mat4 projection = glm::perspective(camera.cameraFOV, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;

		// Model(s) rendering
		gBufferShader.useShader();

		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		GLfloat rotationAngle = glfwGetTime() / 5.0f * modelRotationSpeed;
		model = glm::mat4();
		model = glm::translate(model, modelPosition);
		model = glm::rotate(model, rotationAngle, modelRotationAxis);
		model = glm::scale(model, modelScale);

		projViewModel = projection * view * model;

		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "projViewModel"), 1, GL_FALSE, glm::value_ptr(projViewModel));
		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "prevProjViewModel"), 1, GL_FALSE, glm::value_ptr(prevProjViewModel));
		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		m.Draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		prevProjViewModel = projViewModel;

		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//------------------------
		// Lighting Pass rendering
		//------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightingBRDFShader.useShader();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gNormal);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gEffects);

		glActiveTexture(GL_TEXTURE5);
		envMapHDR.useTexture();
		glActiveTexture(GL_TEXTURE6);
		envMapIrradiance.useTexture();
		glActiveTexture(GL_TEXTURE7);
		envMapPrefilter.useTexture();
		glActiveTexture(GL_TEXTURE8);
		envMapLUT.useTexture();

		glUniformMatrix4fv(glGetUniformLocation(lightingBRDFShader.Program, "inverseView"), 1, GL_FALSE, glm::value_ptr(glm::transpose(view)));
		glUniformMatrix4fv(glGetUniformLocation(lightingBRDFShader.Program, "inverseProj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
		glUniformMatrix4fv(glGetUniformLocation(lightingBRDFShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "gBufferView"), gBufferView);
		glUniform1i(glGetUniformLocation(lightingBRDFShader.Program, "iblMode"), iblMode);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		quadRender.drawShape();

		//-----------------------
		// Forward Pass rendering
		//-----------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		// Copy the depth informations from the Geometry Pass into the default framebuffer
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//----------------
		// ImGui rendering
		//----------------
		ImGui::Render();

		glfwSwapBuffers(window);
	}
	
	//---------
	// Cleaning
	//---------
	ImGui_ImplGlfwGL3_Shutdown();
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

void Engine::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

void Engine::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {

}

void gBufferSetup()
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// Albedo + Roughness
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gAlbedo, 0);

	// Normals + Metalness
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gNormal, 0);

	// Effects (AO + Velocity)
	glGenTextures(1, &gEffects);
	glBindTexture(GL_TEXTURE_2D, gEffects);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gEffects, 0);

	// Define the COLOR_ATTACHMENTS for the G-Buffer
	GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	// Z-Buffer
	glGenRenderbuffers(1, &zBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, zBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zBuffer);

	// Check if the framebuffer is complete before continuing
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete !" << std::endl;
}


void postprocessSetup()
{
	// Post-processing Buffer
	glGenFramebuffers(1, &postprocessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);

	glGenTextures(1, &postprocessBuffer);
	glBindTexture(GL_TEXTURE_2D, postprocessBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Postprocess Framebuffer not complete !" << std::endl;
}


void iblSetup()
{
	// Latlong to Cubemap conversion
	glGenFramebuffers(1, &envToCubeFBO);
	glGenRenderbuffers(1, &envToCubeRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envToCubeFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, envToCubeRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, envMapCube.getTexWidth(), envMapCube.getTexHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, envToCubeRBO);

	latlongToCubeShader.useShader();

	glUniformMatrix4fv(glGetUniformLocation(latlongToCubeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(envMapProjection));
	glActiveTexture(GL_TEXTURE0);
	envMapHDR.useTexture();

	glViewport(0, 0, envMapCube.getTexWidth(), envMapCube.getTexHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, envToCubeFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(latlongToCubeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(envMapView[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMapCube.getTexID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		envCubeRender.drawShape();
	}

	envMapCube.computeTexMipmap();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Diffuse irradiance capture
	glGenFramebuffers(1, &irradianceFBO);
	glGenRenderbuffers(1, &irradianceRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, irradianceFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, irradianceRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, envMapIrradiance.getTexWidth(), envMapIrradiance.getTexHeight());

	irradianceIBLShader.useShader();

	glUniformMatrix4fv(glGetUniformLocation(irradianceIBLShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(envMapProjection));
	glActiveTexture(GL_TEXTURE0);
	envMapCube.useTexture();

	glViewport(0, 0, envMapIrradiance.getTexWidth(), envMapIrradiance.getTexHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, irradianceFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(irradianceIBLShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(envMapView[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMapIrradiance.getTexID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		envCubeRender.drawShape();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Prefilter cubemap
	prefilterIBLShader.useShader();

	glUniformMatrix4fv(glGetUniformLocation(prefilterIBLShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(envMapProjection));
	envMapCube.useTexture();

	glGenFramebuffers(1, &prefilterFBO);
	glGenRenderbuffers(1, &prefilterRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, prefilterFBO);

	unsigned int maxMipLevels = 5;

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = envMapPrefilter.getTexWidth() * std::pow(0.5, mip);
		unsigned int mipHeight = envMapPrefilter.getTexHeight() * std::pow(0.5, mip);

		glBindRenderbuffer(GL_RENDERBUFFER, prefilterRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);

		glUniform1f(glGetUniformLocation(prefilterIBLShader.Program, "roughness"), roughness);
		glUniform1f(glGetUniformLocation(prefilterIBLShader.Program, "cubeResolutionWidth"), envMapPrefilter.getTexWidth());
		glUniform1f(glGetUniformLocation(prefilterIBLShader.Program, "cubeResolutionHeight"), envMapPrefilter.getTexHeight());

		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(prefilterIBLShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(envMapView[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMapPrefilter.getTexID(), mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			envCubeRender.drawShape();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// BRDF LUT
	glGenFramebuffers(1, &brdfLUTFBO);
	glGenRenderbuffers(1, &brdfLUTRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, brdfLUTFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, brdfLUTRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, envMapLUT.getTexWidth(), envMapLUT.getTexHeight());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, envMapLUT.getTexID(), 0);

	glViewport(0, 0, envMapLUT.getTexWidth(), envMapLUT.getTexHeight());
	integrateIBLShader.useShader();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	quadRender.drawShape();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, WIDTH, HEIGHT);
}
