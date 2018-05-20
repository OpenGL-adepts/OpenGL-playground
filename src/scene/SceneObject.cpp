#include "SceneObject.hpp"


void SceneObject::Draw()
{
	m_model.Draw();
}


void SceneObject::Load(const std::string& _path)
{
	m_modelPath = _path;
	m_model.loadModel(m_modelPath);
}


const std::string& SceneObject::GetModelPath() const
{
	return m_modelPath;
}
