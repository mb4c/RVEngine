#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <memory>
#include <stb_image.h>
#include "Shader.hpp"
#include "ResourceManager.hpp"

class EnvironmentMap
{
public:
	EnvironmentMap(const std::string& hdriPath);

	void Capture();
	uint32_t envCubemap;
	uint32_t hdrTexture;
	uint32_t irradianceMap;
	uint32_t prefilterMap;
	uint32_t brdfLUTTexture;
private:
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[6] =
			{
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 1.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
			};

	uint32_t cubeVAO = 0;
	uint32_t cubeVBO = 0;
	uint32_t quadVAO = 0;
	uint32_t quadVBO = 0;

	uint32_t captureFBO;
	uint32_t captureRBO;



	void RenderCube();
	void RenderQuad();
};
