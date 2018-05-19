#pragma once
#include "SceneObject.hpp"
#include <string>
#include <vector>


class Scene
{
public:
	bool Load(const std::string& _path);
	bool Save(const std::string& _path);

	void Draw();

protected:
	std::vector<SceneObject> m_objects;

};
