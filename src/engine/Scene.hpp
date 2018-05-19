#pragma once
#include "model.hpp"
#include <string>


class Scene
{
public:
	bool Load(const std::string& _path);
	bool Save(const std::string& _path);

	void Draw();

protected:
	// One model for now
	Model m_model;
	std::string m_modelPath;
};
