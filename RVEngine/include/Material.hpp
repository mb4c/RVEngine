#pragma once

#include <memory>
#include "Texture.hpp"
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
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normal;
	std::shared_ptr<Texture> occlusionRoughnessMetallic;

	glm::vec4 albedoColor = {1,1,1,1};
	float metallicValue = 0;

};
