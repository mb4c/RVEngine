#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "VertexArray.hpp"
#include "Texture.hpp"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;

};



class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
//	Mesh();

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<Texture> m_Textures;
	std::shared_ptr<VertexArray> m_VertexArray;
};

