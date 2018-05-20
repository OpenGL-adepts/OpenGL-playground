#pragma once
#include "model.hpp"
#include <string>


class SceneObject
{
public:
	void Draw();
	void Load(const std::string& _path);

	const std::string& GetModelPath() const;

protected:
	Model m_model;
	std::string m_modelPath;
};
