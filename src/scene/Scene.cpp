#include "Scene.hpp"
#include <json.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>


bool Scene::Load(const std::string& _path)
{
	std::ifstream file(_path);
	m_objects.clear();

	int debugOffset = 0;

	try
	{
		nlohmann::json json;
		file >> json;

		if(!file)
			return false;

		auto root = std::filesystem::path(_path).parent_path();

		for(auto& elem : json.at("objects"))
		{
			SceneObject tmpObj;

			//TODO: gdzies to walnac indziej:
			tmpObj.setPosition(glm::vec3(debugOffset++ * 1.f, 0, 0));
			/////

			tmpObj.Load((root / elem.at("model").get<std::string>()).make_preferred().string());
			m_objects.push_back(std::move(tmpObj));
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}


bool Scene::Save(const std::string& _path)
{
	nlohmann::json json;
	auto& objArr = json["objects"];

	for(auto& obj : m_objects)
	{
		nlohmann::json tmp;
		tmp["model"] = std::filesystem::relative(obj.GetModelPath(), std::filesystem::path(_path).parent_path()).string();
		objArr.push_back(tmp);
	}

	std::ofstream file(_path);
	file << json;
	return !!file;
}


void Scene::Draw(Shader& _bufferShader, glm::mat4 _projViewMat)
{
	for (auto& obj : m_objects)
	{
		auto model = obj.getModelMatrix();
		auto projViewModel = _projViewMat * model;

		glUniformMatrix4fv(glGetUniformLocation(_bufferShader.Program, "projViewModel"), 1, GL_FALSE, glm::value_ptr(projViewModel));
		glUniformMatrix4fv(glGetUniformLocation(_bufferShader.Program, "prevProjViewModel"), 1, GL_FALSE, glm::value_ptr(projViewModel)); //TODO: prevProjViewModel, ale nie wiem po co to by³o :/
		glUniformMatrix4fv(glGetUniformLocation(_bufferShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		obj.Draw();
	}
}
