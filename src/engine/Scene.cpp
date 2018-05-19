#include "Scene.hpp"
#include <json.hpp>
#include <string>
#include <fstream>


bool Scene::Load(const std::string& _path)
{
	std::ifstream file(_path);

	try
	{
		nlohmann::json json;
		file >> json;

		if(!file)
			return false;

		m_modelPath = json.at("model").get<std::string>();
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
	json["model"] = m_modelPath;

	std::ofstream file(_path);
	file << json;
	return !!file;
}


void Scene::Draw()
{
	m_model.Draw();
}
