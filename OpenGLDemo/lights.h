#pragma once
struct DirLight
{
	glm::vec3 direction;
	glm::vec3 color;
};

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;
};

struct SpotLight {
	glm::vec3 position;
	glm::vec3 direction;
	float edgeCoeff;
	glm::vec3 color;
};