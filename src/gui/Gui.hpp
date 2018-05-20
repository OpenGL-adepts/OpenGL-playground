#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <filesystem>


class Gui
{
public:
	void initGui(GLFWwindow* window);
	void updateGui();
	std::filesystem::path openModelDialog();

private:
	void createImGuiMenu();

protected:
	GLFWwindow* m_window;

};
