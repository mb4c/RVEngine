#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <Texture2D.hpp>
#include <Macros.hpp>
#include <YAMLUtils.hpp>
#include <fstream>
#include "ResourceManager.hpp"

Texture2D::Texture2D(const std::string& path, bool normalMap, bool nearestFiltering)
{
	m_ID = FromFile(path, normalMap, nearestFiltering);
	m_Path = path;
}

uint32_t Texture2D::FromFile(const std::string& path, bool normalMap, bool nearestFiltering)
{
	RV_PROFILE_FUNCTION();


	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		if (normalMap)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

		if (nearestFiltering)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}


		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load: " << (!path.empty() ? path : "No path!") << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void Texture2D::SetTexture(uint32_t id)
{
	m_ID = id;
}

uint32_t Texture2D::GetTexture() const
{
	return m_ID;
}

Texture2D::Texture2D(uint32_t width, uint32_t height, glm::vec4 color, bool normalMap, bool nearestFiltering)
{
	RV_PROFILE_FUNCTION();

	unsigned int textureID;
	glGenTextures(1, &textureID);
	unsigned char pixelData[width * height * 4];

	for (int i = 0; i < width * height; ++i)
	{
		pixelData[i * 4] = static_cast<unsigned char>(color.r * 255);
		pixelData[i * 4 + 1] = static_cast<unsigned char>(color.g * 255);
		pixelData[i * 4 + 2] = static_cast<unsigned char>(color.b * 255);
		pixelData[i * 4 + 3] = static_cast<unsigned char>(color.a * 255);
	}

	auto format = GL_RGBA;

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixelData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	if (nearestFiltering)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}


	m_ID = textureID;
}

void Texture2D::Serialize(const std::filesystem::path& file)
{

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Texture2D" << YAML::Value << m_UUID;
	out << YAML::Key << "IsNormalMap" << YAML::Value << m_IsNormalMap;
	out << YAML::Key << "RelativePath" << YAML::Value << m_RelativePath;

	out << YAML::EndMap;

	std::ofstream fout(file);
	fout << out.c_str();
}

void Texture2D::Deserialize(const std::filesystem::path& file)
{
	ResourceManager& rm = ResourceManager::instance();

	std::ifstream stream(file);
	std::stringstream strStream;
	strStream << stream.rdbuf();

	YAML::Node data = YAML::Load(strStream.str());
	if (!data["Texture2D"])
		std::cout << "No data!" << std::endl;

	m_UUID = data["Texture2D"].as<uint64_t>();
	m_IsNormalMap = data["IsNormalMap"].as<bool>();
	m_RelativePath = data["RelativePath"].as<std::string>();
}


