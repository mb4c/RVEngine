#include <Mesh.hpp>
#include <Macros.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture2D> textures)
{
	RV_PROFILE_FUNCTION();

	this->m_Vertices = vertices;
	this->m_Indices = indices;
	this->m_Textures = textures;

	std::vector<float> interleavedVertices;
	for (int i = 0; i < m_Vertices.size(); ++i)
	{
		interleavedVertices.push_back(m_Vertices[i].Position.x);
		interleavedVertices.push_back(m_Vertices[i].Position.y);
		interleavedVertices.push_back(m_Vertices[i].Position.z);
		interleavedVertices.push_back(m_Vertices[i].Normal.x);
		interleavedVertices.push_back(m_Vertices[i].Normal.y);
		interleavedVertices.push_back(m_Vertices[i].Normal.z);
		interleavedVertices.push_back(m_Vertices[i].TexCoords.x);
		interleavedVertices.push_back(m_Vertices[i].TexCoords.y);
		interleavedVertices.push_back(m_Vertices[i].Tangent.x);
		interleavedVertices.push_back(m_Vertices[i].Tangent.y);
		interleavedVertices.push_back(m_Vertices[i].Tangent.z);
		interleavedVertices.push_back(m_Vertices[i].Bitangent.x);
		interleavedVertices.push_back(m_Vertices[i].Bitangent.y);
		interleavedVertices.push_back(m_Vertices[i].Bitangent.z);
	}

	std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(interleavedVertices.data(), interleavedVertices.size() * sizeof(float));
	BufferLayout layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Bitangent" }

		};
	vertexBuffer->SetLayout(layout);

	m_VertexArray = std::make_shared<VertexArray>();
	m_VertexArray->AddVertexBuffer(vertexBuffer);

	std::shared_ptr<IndexBuffer> indexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size()); // * sizeof(uint32_t)?
	m_VertexArray->SetIndexBuffer(indexBuffer);

}

