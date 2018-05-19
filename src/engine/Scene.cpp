#include "Scene.hpp"
#include <json.hpp>
#include <string>
#include <fstream>
#include <filesystem>


bool Scene::Load(const std::string& _path)
{
	std::ifstream file(_path);

	try
	{
		nlohmann::json json;
		file >> json;

		if(!file)
			return false;

		auto root = std::filesystem::path(_path).parent_path();

		m_modelPath = (root / json.at("model").get<std::string>()).string();
		m_model.loadModel(m_modelPath);
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
	json["model"] = std::filesystem::relative(m_modelPath, std::filesystem::path(_path).parent_path()).string();

	std::ofstream file(_path);
	file << json;
	return !!file;
}


void Scene::Draw()
{
	m_model.Draw();
}
