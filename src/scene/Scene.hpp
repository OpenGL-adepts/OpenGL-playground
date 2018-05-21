#pragma once
#include "../processing/shader.hpp"
#include "SceneObject.hpp"
#include <string>
#include <vector>


class Scene
{
public:
	bool Load(const std::string& _path);
	bool Save(const std::string& _path);

	void Draw(Shader& _bufferShader, glm::mat4 _projViewMat);

	void setRotationAll(float _angle)
	{
		for (auto& obj : m_objects)
			obj.setRotation(_angle);
	}

protected:
	std::vector<SceneObject> m_objects;

};
