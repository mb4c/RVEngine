#pragma once

#include <memory>
#include "Texture2D.hpp"
#include "Shader.hpp"
#include "UUID.hpp"
#include <filesystem>

class Material
{
public:
	Material();
	void Serialize(const std::filesystem::path& file);
	void Deserialize(const std::filesystem::path& file);

	std::string materialName;
	UUID uuid;
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Texture2D> albedo;
	std::shared_ptr<Texture2D> normal;
	std::shared_ptr<Texture2D> occlusion;
	std::shared_ptr<Texture2D> roughness;
	std::shared_ptr<Texture2D> metallic;
	std::shared_ptr<Texture2D> emission;

	bool useAlbedo = true;
	bool useNormal = true;
	bool useOcclusion = true;
	bool useRoughness = true;
	bool useMetallic = true;
	bool useEmission = true;

	glm::vec4 albedoColor = {1,1,1,1};
	float roughnessValue = 0;
	float metallicValue = 0;
	float emissionStrength = 1;

};
