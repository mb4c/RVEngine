#pragma once

#include <memory>
#include "Texture2D.hpp"
#include "Shader.hpp"
#include <filesystem>

class Material
{
public:
	Material();
	void Serialize(std::filesystem::path file);
	void Deserialize(const std::filesystem::path& file);

	std::string materialName;
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Texture2D> albedo;
	std::shared_ptr<Texture2D> normal;
	std::shared_ptr<Texture2D> occlusionRoughnessMetallic;

	glm::vec4 albedoColor = {1,1,1,1};
	float metallicValue = 0;

};
