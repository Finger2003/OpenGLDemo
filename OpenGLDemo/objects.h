#pragma once
#include <learnopengl/model.h>

class Object
{
	Model model;
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat3 normalModelMatrix = glm::mat3(1.0f);

public:
	Object(Model model) : model(model) {}

	void SetModelMatrix(glm::mat4 model)
	{
		modelMatrix = model;
		normalModelMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
	}
	void Draw(Shader& shader)
	{
		shader.setMat4("model", modelMatrix);
		shader.setMat3("normalModel", normalModelMatrix);
		model.Draw(shader);
	}
};


class LightObject : public Object
{
public:
	LightObject(Model model) : Object(model) {}
	glm::vec3 lightPositionOffset = glm::vec3(0.0f);
};


class SpotlightObject : public LightObject
{
public:
	SpotlightObject(Model model) : LightObject(model) {}
	glm::vec3 lightDirection = glm::vec3(0.0f);
};