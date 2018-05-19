#include "Scene.hpp"
#include <json.hpp>
#include <string>
#include <fstream>
#include <filesystem>


bool Scene::Load(const std::string& _path)
{
	std::ifstream file(_path);
	m_objects.clear();

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


void Scene::Draw()
{
	for(auto& obj : m_objects)
		obj.Draw();
}
