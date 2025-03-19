#include <learnopengl/shader.h>

class Mirror
{
	unsigned int VAO;
public:
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	Mirror(float vertices[], int size)
	{
		unsigned int VBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void Draw(Shader& shader)
	{
		shader.setMat4("model", modelMatrix);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};