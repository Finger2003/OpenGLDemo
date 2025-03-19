#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#pragma warning(pop)

#include <iostream>

#include "objects.h"
#include "skybox.h"
#include "mirror.h"
#include "lights.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(const std::vector<std::string>& faces);
void setWindowTitle(GLFWwindow* window);
void drawSkybox(Skybox& skybox, Shader& shader, unsigned int cubemapTexture, glm::mat4 view, glm::mat4 projection);
glm::vec3 calculateFlashlightPositionAndAngle(float time, float& angle);
void setCameras(const glm::mat4& flashLightModel);
void drawObjects(Shader& shader, const std::vector<Object*>& objects);

void drawScene(Shader& lightingShader, Shader& skyboxShader, Skybox& skybox, const vector<Object*>& objects, const glm::mat4& view, const glm::mat4& projection, glm::vec3 viewPos, unsigned int cubemapTexture);
glm::mat4 setFlashlight(SpotlightObject& flashlight, SpotLight& spotlight, float currentFrame);

// settings
unsigned int SCR_WIDTH = 1600;
unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// planes
float nearPlane = 0.1f;
float farPlane = 100.0f;

// free camera
Camera freeCamera;
// still camera
Camera stillCamera(glm::vec3(-10.0f, 4.0f, 12.0f));
// still following camera
Camera pointedCamera(glm::vec3(8.0f, 4.0f, 9.0f));
// attached camera
Camera attachedCamera(glm::vec3(0.0f, 0.0f, 0.0f));

Camera* activeCamera = &stillCamera;
std::vector<Camera*> cameras = { &stillCamera, &pointedCamera, &attachedCamera, &freeCamera };
size_t activeCameraIndex = 0;

bool isDay = true;
bool useBlinn = false;
float fogIntensity = 0.0f;
glm::vec3 fogColor = glm::vec3(0.8f);
float relativeReflectorAngleX = 0.0f;
float relativeReflectorAngleY = 0.0f;




int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// build and compile shaders
	// -------------------------
	Shader skyboxShader("Shaders/skybox_shader.vert", "Shaders/skybox_shader.frag");
	Shader lightingShader("Shaders/lighting_shader.vert", "Shaders/lighting_shader.frag");
	Shader constantShader("Shaders/constant_shader.vert", "Shaders/constant_shader.frag");

	// load models
	// -----------
	Model sphereModel(FileSystem::getPath("Resources/objects/sphere/sphere.obj"));
	Model lanternModel(FileSystem::getPath("Resources/objects/lantern/lantern.obj"));
	Model flashlightModel(FileSystem::getPath("Resources/objects/flashlight/flashlight.obj"));
	Model floorModel(FileSystem::getPath("Resources/objects/floor/floor.obj"));
	Model houseModel(FileSystem::getPath("Resources/objects/house/house.obj"));

	Object sphere(sphereModel);
	Object floor(floorModel);
	Object house(houseModel);


	LightObject lantern(lanternModel);
	//lantern.position = glm::vec3(1.0f, 0.0f, 5.0f);
	lantern.lightPositionOffset = glm::vec3(0.0f, 460.0f, 0.0f);
	constexpr glm::vec3 lanternPosition = glm::vec3(1.0f, 0.0f, 5.0f);

	SpotlightObject flashlight(flashlightModel);
	flashlight.lightPositionOffset = glm::vec3(0.0f, -0.004f, 0.08f);
	flashlight.lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	std::vector<Object*> objects = { &flashlight, &sphere, &lantern, &house, &floor };


	// load skybox
	// -----------
	Skybox skybox;

	vector<std::string> dayFaces
	{
		FileSystem::getPath("Resources/textures/skybox/right.jpg"),
		FileSystem::getPath("Resources/textures/skybox/left.jpg"),
		FileSystem::getPath("Resources/textures/skybox/top.jpg"),
		FileSystem::getPath("Resources/textures/skybox/bottom.jpg"),
		FileSystem::getPath("Resources/textures/skybox/front.jpg"),
		FileSystem::getPath("Resources/textures/skybox/back.jpg")
	};

	vector<std::string> nightFaces
	{
		FileSystem::getPath("Resources/textures/night_skybox/right.png"),
		FileSystem::getPath("Resources/textures/night_skybox/left.png"),
		FileSystem::getPath("Resources/textures/night_skybox/top.png"),
		FileSystem::getPath("Resources/textures/night_skybox/bottom.png"),
		FileSystem::getPath("Resources/textures/night_skybox/front.png"),
		FileSystem::getPath("Resources/textures/night_skybox/back.png")
	};

	unsigned int cubemapDayTexture = loadCubemap(dayFaces);
	unsigned int cubemapNightTexture = loadCubemap(nightFaces);



	// mirror vertex data
	float mirrorVertices[] = {
		// positions        
		-1.4f, -1.0f, 0.0f,
		-1.4f, 1.0f, 0.0f,
		 1.4f,  1.0f, 0.0f,

		 1.4f,  1.0f, 0.0f,
		 1.4f, -1.0f, 0.0f,
		 -1.4f, -1.0f, 0.0f,
	};

	Mirror mirror(mirrorVertices, sizeof(mirrorVertices));


	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::mat4 model, view, projection, reflectedProjection;
	projection = glm::perspective(glm::radians(activeCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearPlane, farPlane);
	reflectedProjection = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 1.0f)) * projection;


	DirLight dirLight;
	dirLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	dirLight.color = glm::vec3(0.5);

	PointLight pointLight;
	glm::mat4 pointLightMatrix = glm::translate(glm::mat4(1), lanternPosition);
	pointLightMatrix = glm::scale(pointLightMatrix, glm::vec3(0.005f));
	pointLight.position = glm::vec3(pointLightMatrix * glm::vec4(lantern.lightPositionOffset, 1.0f));
	pointLight.color = glm::vec3(1.0f);

	SpotLight spotLight;
	spotLight.color = glm::vec3(1.0f);
	spotLight.edgeCoeff = 50;


	// sphere model	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0f, 1.0f, 9.5f));
	sphere.SetModelMatrix(model);

	// lantern model
	model = glm::mat4(1.0f);
	model = glm::translate(model, lanternPosition);
	model = glm::scale(model, glm::vec3(0.005f));
	lantern.SetModelMatrix(model);

	// floor model
	floor.SetModelMatrix(glm::mat4(1.0f));

	// house model
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(7.0f, 0.0f, 10.0f));
	model = glm::rotate(model, glm::radians(225.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	house.SetModelMatrix(model);


	// mirror model
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(3.0f));
	mirror.modelMatrix = model;


	// set light properties
	lightingShader.use();
	lightingShader.setVec3("dirLight.direction", dirLight.direction);
	lightingShader.setVec3("dirLight.color", dirLight.color);
	lightingShader.setVec3("pointLights[0].position", pointLight.position);
	lightingShader.setVec3("pointLights[0].color", pointLight.color);
	lightingShader.setFloat("spotLights[0].edgeCoeff", spotLight.edgeCoeff);
	lightingShader.setVec3("spotLights[0].color", spotLight.color);

	// set cameras
	stillCamera.SetFront(glm::normalize(glm::vec3(0.8f, -0.2f, -0.5f)));
	freeCamera = stillCamera;


	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		projection = glm::perspective(glm::radians(activeCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		reflectedProjection = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 1.0f)) * projection;


		// set flashlight
		model = setFlashlight(flashlight, spotLight, currentFrame);		

		// set cameras
		setCameras(model);


		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x00);

		lightingShader.use();

		// set global uniforms
		lightingShader.setVec3("spotLights[0].position", spotLight.position);
		lightingShader.setVec3("spotLights[0].direction", spotLight.direction);
		lightingShader.setBool("isDay", isDay);
		lightingShader.setBool("useBlinn", useBlinn);
		lightingShader.setFloat("fogIntensity", fogIntensity);
		lightingShader.setVec3("fogColor", fogColor);

		view = activeCamera->GetViewMatrix();

		unsigned int cubemapTexture = isDay ? cubemapDayTexture : cubemapNightTexture;
		drawScene(lightingShader, skyboxShader, skybox, objects, view, projection, activeCamera->Position, cubemapTexture);


		// RENDER MIRROR
		// -------------
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		constantShader.use();
		constantShader.setMat4("projection", projection);
		constantShader.setMat4("view", view);

		mirror.Draw(constantShader);

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glClear(GL_DEPTH_BUFFER_BIT);
		// -------------



		// RENDER REFLECTED OBJECTS
		// ------------------------
		glm::mat4 reflectorMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
		glm::vec3 viewPos = glm::vec3(reflectorMatrix * glm::vec4(activeCamera->Position, 1.0f));
		glm::vec3 viewDir = glm::vec3(reflectorMatrix * glm::vec4(activeCamera->Front, 0.0f));
		glm::vec3 viewUp = glm::vec3(reflectorMatrix * glm::vec4(activeCamera->Up, 0.0f));

		view = glm::lookAt(viewPos, viewPos + viewDir, viewUp);

		lightingShader.use();
		drawScene(lightingShader, skyboxShader, skybox, objects, view, reflectedProjection, viewPos, cubemapTexture);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		// ------------------------

		// set windows title with options
		setWindowTitle(window);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	static bool isPressed = false;
	// close application
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// move camera
	if (activeCamera == &freeCamera)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			freeCamera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			freeCamera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			freeCamera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			freeCamera.ProcessKeyboard(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			freeCamera.ProcessKeyboard(UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			freeCamera.ProcessKeyboard(DOWN, deltaTime);
	}

	freeCamera.Position.z = glm::max(freeCamera.Position.z, 0.05f);

	// change fog intensity
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		fogIntensity = glm::max(0.0f, fogIntensity - 0.01f);
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		fogIntensity = glm::min(1.0f, fogIntensity + 0.01f);

	// change reflector angle
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		relativeReflectorAngleY = glm::max(-1.0f, relativeReflectorAngleY - 0.01f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		relativeReflectorAngleY = glm::min(1.0f, relativeReflectorAngleY + 0.01f);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		relativeReflectorAngleX = glm::min(1.0f, relativeReflectorAngleX + 0.01f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		relativeReflectorAngleX = glm::max(-1.0f, relativeReflectorAngleX - 0.01f);

	// change camera
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !isPressed)
	{
		activeCameraIndex = activeCameraIndex == 0 ? cameras.size() - 1 : activeCameraIndex - 1;
		activeCamera = cameras[activeCameraIndex];
		isPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !isPressed)
	{
		activeCameraIndex = (activeCameraIndex + 1) % cameras.size();
		activeCamera = cameras[activeCameraIndex];
		isPressed = true;
	}

	// change day/night
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !isPressed)
	{
		isDay = !isDay;
		isPressed = true;
	}

	// change lighting model
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !isPressed)
	{
		useBlinn = !useBlinn;
		isPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE &&
		glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
		isPressed = false;
}

unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap(const std::vector<std::string>& faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format = 0;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void setWindowTitle(GLFWwindow* window)
{
	std::string title = "Lab 4 - ";
	title += isDay ? "Day" : "Night";
	title += " - ";
	title += useBlinn ? "Blinn-Phong" : "Phong";
	title += " - Fog: ";
	title += std::format("{:.2f}", fogIntensity);
	title += " - Camera: ";
	title += activeCamera == &stillCamera ? "Still" : activeCamera == &pointedCamera ? "Pointed" : activeCamera == &attachedCamera ? "Attached" : "Free";
	glfwSetWindowTitle(window, title.c_str());
}

void drawSkybox(Skybox& skybox, Shader& shader, unsigned int cubemapTexture, glm::mat4 view, glm::mat4 projection)
{
	shader.use();
	shader.setBool("isFoggy", fogIntensity > 0.0f);
	shader.setVec3("fogColor", fogColor);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	skybox.Draw(shader, cubemapTexture);
}

glm::vec3 calculateFlashlightPositionAndAngle(float time, float& angle)
{
	constexpr float A = 5.0f;
	float x = A * glm::sin(time);
	float z = x * glm::cos(time);


	float dx = A * glm::cos(time);
	float dz = A * glm::cos(2 * time);

	angle = glm::atan(dx, dz);
	return glm::vec3(x, 0.0f, z);
}

void setCameras(const glm::mat4& flashLightModel)
{
	// set attached camera position
	constexpr glm::vec3 attachedCameraOffset = glm::vec3(0.0f, 0.3f, -0.5f);

	const glm::vec3& flashLightPosition = glm::vec3(flashLightModel * glm::vec4(glm::vec3(0.0f), 1.0f));

	attachedCamera.Position = glm::vec3(flashLightModel * glm::vec4(attachedCameraOffset, 1.0f));
	attachedCamera.Front = glm::normalize(flashLightPosition - attachedCamera.Position);


	// set observing camera target
	pointedCamera.Front = glm::normalize(flashLightPosition - pointedCamera.Position);
}

void drawObjects(Shader& shader, const std::vector<Object*>& objects)
{
	for (Object* object : objects)
		object->Draw(shader);
}

void drawScene(Shader& lightingShader, Shader& skyboxShader, Skybox& skybox, const vector<Object*>& objects, const glm::mat4& view, const glm::mat4& projection, glm::vec3 viewPos, unsigned int cubemapTexture)
{
	// set observer position
	lightingShader.setVec3("viewPos", viewPos);

	// view/projection transformations
	lightingShader.setMat4("projection", projection);
	lightingShader.setMat4("view", view);

	// render objects
	drawObjects(lightingShader, objects);

	// draw skybox as last
	drawSkybox(skybox, skyboxShader, cubemapTexture, glm::mat4(glm::mat3(view)), projection);
}

glm::mat4 setFlashlight(SpotlightObject& flashlight, SpotLight& spotlight, float currentFrame)
{
	// calculate moving objects positions
	float angle;
	glm::vec3 flashLightPosition = calculateFlashlightPositionAndAngle(currentFrame * 0.5f, angle);

	// flashLight model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0f, 0.5f, 6.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, flashLightPosition);
	model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, relativeReflectorAngleX, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, relativeReflectorAngleY, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f));
	flashlight.SetModelMatrix(model);

	// set spotlight properties
	spotlight.position = glm::vec3(model * glm::vec4(flashlight.lightPositionOffset, 1.0f));
	spotlight.direction = glm::vec3(model * glm::vec4(flashlight.lightDirection, 0.0f));

	return model;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);

	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (activeCamera != &freeCamera)
		return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	freeCamera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	activeCamera->ProcessMouseScroll(static_cast<float>(yoffset));
}






