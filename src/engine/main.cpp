#include "../processing/shader.hpp"
#include "../camera/Camera.hpp"
#include "../model/Model.hpp"
#include "../processing/shape.hpp"
#include "../processing/texture.hpp"
#include "../processing/light.hpp"
#include "../processing/skybox.hpp"
#include "../processing/material.hpp"
#include "../scene/Scene.hpp"

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


//---------------
// GLFW Callbacks
//---------------

static void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//---------------------
// Functions prototypes
//---------------------

void cameraMove();
void imGuiSetup();
void gBufferSetup();
void postprocessSetup();
void iblSetup();

//---------------------------------
// Variables & objects declarations
//---------------------------------

GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

GLuint screenQuadVAO, screenQuadVBO;
GLuint gBuffer, zBuffer, gPosition, gNormal, gAlbedo, gEffects;
GLuint postprocessFBO, postprocessBuffer;
GLuint envToCubeFBO, irradianceFBO, prefilterFBO, brdfLUTFBO, envToCubeRBO, irradianceRBO, prefilterRBO, brdfLUTRBO;

GLint gBufferView = 1;
GLint tonemappingMode = 1;
GLint lightDebugMode = 3;
GLint attenuationMode = 2;
GLint motionBlurMaxSamples = 32;

GLfloat lastX = WIDTH / 2;
GLfloat lastY = HEIGHT / 2;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat deltaGeometryTime = 0.0f;
GLfloat deltaLightingTime = 0.0f;
GLfloat deltaPostprocessTime = 0.0f;
GLfloat deltaForwardTime = 0.0f;
GLfloat deltaGUITime = 0.0f;
GLfloat materialRoughness = 0.01f;
GLfloat materialMetallicity = 0.02f;
GLfloat ambientIntensity = 0.005f;
GLfloat lightPointRadius1 = 3.0f;
GLfloat lightPointRadius2 = 3.0f;
GLfloat lightPointRadius3 = 3.0f;
GLfloat cameraAperture = 16.0f;
GLfloat cameraShutterSpeed = 0.5f;
GLfloat cameraISO = 1000.0f;
GLfloat modelRotationSpeed = 0.0f;

bool cameraMode;
bool pointMode = false;
bool directionalMode = false;
bool iblMode = true;
bool fxaaMode = false;
bool motionBlurMode = false;
bool screenMode = false;
bool firstMouse = true;
bool guiIsOpen = true;
bool keys[1024];

glm::vec3 albedoColor = glm::vec3(1.0f);
glm::vec3 materialF0 = glm::vec3(0.04f);  // UE4 dielectric
glm::vec3 lightPointPosition1 = glm::vec3(1.5f, 0.75f, 1.0f);
glm::vec3 lightPointPosition2 = glm::vec3(-1.5f, 1.0f, 1.0f);
glm::vec3 lightPointPosition3 = glm::vec3(0.0f, 0.75f, -1.2f);
glm::vec3 lightPointColor1 = glm::vec3(1.0f);
glm::vec3 lightPointColor2 = glm::vec3(1.0f);
glm::vec3 lightPointColor3 = glm::vec3(1.0f);
glm::vec3 lightDirectionalDirection1 = glm::vec3(-0.2f, -1.0f, -0.3f);
glm::vec3 lightDirectionalColor1 = glm::vec3(1.0f);
glm::vec3 modelPosition = glm::vec3(0.0f);
glm::vec3 modelRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 modelScale = glm::vec3(0.1f);

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

Shader gBufferShader;
Shader latlongToCubeShader;
Shader simpleShader;
Shader lightingBRDFShader;
Shader irradianceIBLShader;
Shader prefilterIBLShader;
Shader integrateIBLShader;
Shader firstpassPPShader;

Texture objectAlbedo;
Texture objectNormal;
Texture objectRoughness;
Texture objectMetalness;
Texture objectAO;
Texture envMapHDR;
Texture envMapCube;
Texture envMapIrradiance;
Texture envMapPrefilter;
Texture envMapLUT;

Model objectModel;

Light lightPoint1;
Light lightPoint2;
Light lightPoint3;
Light lightDirectional1;

Shape quadRender;
Shape envCubeRender;
Scene scene;

void loadScene() {
	scene.Load("C:/Users/Kasia/OpenGL-playground/resources/scene.json");
}


int main(int argc, char* argv[])
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* glfwMode = glfwGetVideoMode(glfwMonitor);

	glfwWindowHint(GLFW_RED_BITS, glfwMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, glfwMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, glfwMode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, glfwMode->refreshRate);

	WIDTH = glfwMode->width;
	HEIGHT = glfwMode->height;

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GLEngine", glfwMonitor, NULL);
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSwapInterval(1);

	gladLoadGL();

	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	ImGui_ImplGlfwGL3_Init(window, true);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);


	//----------
	// Shader(s)
	//----------
	gBufferShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/gBuffer.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/gBuffer.frag");
	latlongToCubeShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/latlongToCube.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/latlongToCube.frag");

	simpleShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/simple.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/simple.frag");

	lightingBRDFShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/lightingBRDF.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/lightingBRDF.frag");
	irradianceIBLShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/irradianceIBL.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/irradianceIBL.frag");
	prefilterIBLShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/prefilterIBL.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/prefilterIBL.frag");
	integrateIBLShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/integrateIBL.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/lighting/integrateIBL.frag");
	firstpassPPShader.setShader("C:/Users/Kasia/OpenGL-playground/resources/shaders/postprocess/postprocess.vert", "C:/Users/Kasia/OpenGL-playground/resources/shaders/postprocess/firstpass.frag");



	//-----------
	// Textures(s)
	//-----------

	envMapHDR.setTextureHDR("C:/Users/Kasia/OpenGL-playground/resources/textures/hdr/popcorn.hdr", "space", true);

	envMapCube.setTextureCube(512, GL_RGB, GL_RGB16F, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR);
	envMapIrradiance.setTextureCube(32, GL_RGB, GL_RGB16F, GL_FLOAT, GL_LINEAR);
	envMapPrefilter.setTextureCube(128, GL_RGB, GL_RGB16F, GL_FLOAT, GL_LINEAR_MIPMAP_LINEAR);
	envMapPrefilter.computeTexMipmap();
	envMapLUT.setTextureHDR(512, 512, GL_RG, GL_RG16F, GL_FLOAT, GL_LINEAR);


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
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		cameraMove();


		//--------------
		// ImGui setting
		//--------------
		imGuiSetup();

		//------------------------
		// Geometry Pass rendering
		//------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Camera setting
		glm::mat4 projection = glm::perspective(camera.cameraFOV, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;

		// Model(s) rendering
		gBufferShader.useShader();

		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		scene.setRotationAll(glfwGetTime() / 5.0f * modelRotationSpeed);

		//GLfloat rotationAngle = glfwGetTime() / 5.0f * modelRotationSpeed;
		//model = glm::mat4();
		//model = glm::translate(model, modelPosition);
		//model = glm::rotate(model, rotationAngle, modelRotationAxis);
		//model = glm::scale(model, modelScale);

		//projViewModel = projection * view * model;
		//
		//glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "projViewModel"), 1, GL_FALSE, glm::value_ptr(projViewModel));
		//glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "prevProjViewModel"), 1, GL_FALSE, glm::value_ptr(prevProjViewModel));
		//glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		scene.Draw(gBufferShader, projection * view);
		//objectModel.Draw();

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

	return 0;
}


void cameraMove()
{
	if (keys[GLFW_KEY_W])
		camera.keyboardCall(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.keyboardCall(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.keyboardCall(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.keyboardCall(RIGHT, deltaTime);
}


void imGuiSetup()
{
	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::Begin("", &guiIsOpen, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings);
	ImGui::SetWindowSize(ImVec2(350, HEIGHT));

	if (ImGui::Button("Load", ImVec2(100,100))) {
		loadScene();		
	}
	if (ImGui::Button("Spin", ImVec2(100, 100))) {
		modelRotationSpeed = !modelRotationSpeed * 20.f;
	}

	if (ImGui::Button("Beach ", ImVec2(100, 50))) {
		envMapHDR.setTextureHDR("C:/Users/Kasia/OpenGL-playground/resources/textures/hdr/beach.hdr", "beach", true);
	}
	if (ImGui::Button("Road ", ImVec2(100, 50))) {
		envMapHDR.setTextureHDR("C:/Users/Kasia/OpenGL-playground/resources/textures/hdr/road.hdr", "road", true);
	}
	if (ImGui::Button("Cinema ", ImVec2(100, 50))) {
		envMapHDR.setTextureHDR("C:/Users/Kasia/OpenGL-playground/resources/textures/hdr/popcorn.hdr", "popcorn", true);
	}

	ImGui::End();
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


static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
	{
		screenMode = !screenMode;


	}

	if (keys[GLFW_KEY_1])
		gBufferView = 1;

	if (keys[GLFW_KEY_2])
		gBufferView = 2;

	if (keys[GLFW_KEY_3])
		gBufferView = 3;

	if (keys[GLFW_KEY_4])
		gBufferView = 4;

	if (keys[GLFW_KEY_5])
		gBufferView = 5;

	if (keys[GLFW_KEY_6])
		gBufferView = 6;

	if (keys[GLFW_KEY_7])
		gBufferView = 7;

	if (keys[GLFW_KEY_8])
		gBufferView = 8;

	if (keys[GLFW_KEY_9])
		gBufferView = 9;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	if (cameraMode)
		camera.mouseCall(xoffset, yoffset);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		cameraMode = true;
	else
		cameraMode = false;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (cameraMode)
		camera.scrollCall(yoffset);
}
