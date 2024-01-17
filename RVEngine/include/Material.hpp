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
	void Serialize(std::filesystem::path file);
	void Deserialize(const std::filesystem::path& file);

	std::string materialName;
	UUID uuid;
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Texture2D> albedo;
	std::shared_ptr<Texture2D> normal;
	std::shared_ptr<Texture2D> occlusionRoughnessMetallic;

	bool useAlbedo;
	bool useNormal;
	bool useORM;

	glm::vec4 albedoColor = {1,1,1,1};
	float roughnessValue = 0;
	float metallicValue = 0;

};
