#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "VertexArray.hpp"
#include "Texture2D.hpp"

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
	Mesh() = default;
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture2D> textures);
	void SetupBuffers();
	void RecalculateNormals();
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<Texture2D> m_Textures;
	std::shared_ptr<VertexArray> m_VertexArray;
};

