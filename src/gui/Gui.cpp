#include "Gui.hpp"


void Gui::initGui(GLFWwindow* window) {

	// Setup Dear ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window, true);

	// Enable vsync
	glfwSwapInterval(1);

	// Crete menu
	createImGuiMenu();

}


void Gui::createImGuiMenu() {
	ImGui_ImplGlfwGL3_NewFrame();
	ImGui::Text("Hello, word!");

}
