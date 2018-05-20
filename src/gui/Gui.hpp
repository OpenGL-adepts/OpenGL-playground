#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>


class Gui
{
public:
	void initGui(GLFWwindow* window);
	void updateGui();

private:
	void createImGuiMenu();

};
