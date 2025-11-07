#include <glad/gl.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <iostream>
#include <../include/Renderer/Texture2D.hpp>
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

	OIIO::ImageBuf data(path);
	data = data.copy(OIIO::TypeDesc::UCHAR);
	Texture2D tex;

	if (!data.has_error())
	{
		tex = Texture2D(data.spec().width, data.spec().height, data.spec().nchannels, (unsigned char*)data.localpixels());
	}
	else
	{
		std::cout << "Texture failed to load: " << (!path.empty() ? path : "No path!") << std::endl;
	}

	return tex.m_ID;
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

	std::vector<unsigned char> pixelData(width * height * 4);

	for (int i = 0; i < width * height; ++i)
	{
		pixelData[i * 4] = static_cast<unsigned char>(color.r * 255);
		pixelData[i * 4 + 1] = static_cast<unsigned char>(color.g * 255);
		pixelData[i * 4 + 2] = static_cast<unsigned char>(color.b * 255);
		pixelData[i * 4 + 3] = static_cast<unsigned char>(color.a * 255);
	}

	auto tex = Texture2D(width, height, 4, pixelData.data());
	m_ID = tex.GetTexture();
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

Texture2D::Texture2D(uint32_t width, uint32_t height, uint32_t channels, unsigned char* data, bool normalMap, bool nearestFiltering)
{
	GLenum format;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA;

	if (normalMap)
		format = GL_RGBA;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_REPEAT);

	if (nearestFiltering)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	GLfloat maxAniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(16.0f, maxAniso));
	m_ID = textureID;
}


