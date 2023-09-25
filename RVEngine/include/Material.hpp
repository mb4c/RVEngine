#pragma once

#include <memory>
#include "Texture.hpp"
#include "Shader.hpp"

class Material
{
public:
	Material();
	Texture albedo;
	Texture normal;
	Texture occlusionRoughnessMetallic;
	std::shared_ptr<Shader> shader;
};
