#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <Texture.hpp>
#include <Macros.hpp>

Texture::Texture(const std::string& path, bool normalMap)
{
	m_ID = FromFile(path);
	m_Path = path;
}

uint32_t Texture::FromFile(const std::string& path, bool normalMap)
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void Texture::SetTexture(uint32_t id)
{
	m_ID = id;
}

uint32_t Texture::GetTexture() const
{
	return m_ID;
}

Texture::Texture(uint32_t width, uint32_t height, glm::vec4 color, bool normalMap)
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	m_ID = textureID;
}


