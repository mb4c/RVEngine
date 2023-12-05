#pragma once

#include <memory>
#include "Texture.hpp"
#include "Shader.hpp"

class Material
{
public:
	Material();
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normal;
	std::shared_ptr<Texture> occlusionRoughnessMetallic;
	std::shared_ptr<Shader> shader;
};
