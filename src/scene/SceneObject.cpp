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


void SceneObject::setRotation(float _angle)
{
	m_rotation = _angle;
}


void SceneObject::setPosition(glm::vec3 _pos)
{
	m_position = _pos;
}


const std::string& SceneObject::GetModelPath() const
{
	return m_modelPath;
}


glm::mat4 SceneObject::getModelMatrix() const
{
	glm::mat4 model;
	model = glm::translate(model, m_position);
	model = glm::rotate(model, m_rotation, m_rotAxis);
	model = glm::scale(model, m_scale);
	return model;
}
