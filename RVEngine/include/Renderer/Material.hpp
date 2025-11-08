#pragma once

#include <memory>
#include "Renderer/Texture2D.hpp"
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

	bool useAlbedo = false;
	bool useNormal = false;
	bool useOcclusion = false;
	bool useRoughness = false;
	bool useMetallic = false;
	bool useEmission = false;
	bool useIBL = true;

	glm::vec4 albedoColor = {1,1,1,1};
	glm::vec4 emissionColor = {0,0,0,0};
	float roughnessValue = 0.7;
	float metallicValue = 0;
	float emissionStrength = 1;

};
