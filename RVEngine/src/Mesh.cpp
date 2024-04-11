#include <Mesh.hpp>
#include <Macros.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture2D> textures)
{
	RV_PROFILE_FUNCTION();

	this->m_Vertices = vertices;
	this->m_Indices = indices;
	this->m_Textures = textures;

	SetupBuffers();


}

void Mesh::RecalculateNormals()
{
// Zero-out our normal buffer to start from a clean slate.
//	for (int vertex = 0; vertex < m_Vertices.size(); vertex++)
//		vertexNormals[vertex] = Vector3.zero;

	// For each face, compute the face normal, and accumulate it into each vertex.
	for (int index = 0; index < m_Indices.size(); index += 3)
	{
		int vertexA = m_Indices[index];
		int vertexB = m_Indices[index + 1];
		int vertexC = m_Indices[index + 2];

		auto edgeAB = m_Vertices[vertexB].Position - m_Vertices[vertexA].Position;
//			auto edgeAC = vertexPositions[vertexC] - vertexPositions[vertexA];
		auto edgeAC = m_Vertices[vertexC].Position - m_Vertices[vertexA].Position;


		// The cross product is perpendicular to both input vectors (normal to the plane).
		// Flip the argument order if you need the opposite winding.
		auto areaWeightedNormal = glm::cross(edgeAB, edgeAC);

		// Don't normalize this vector just yet. Its magnitude is proportional to the
		// area of the triangle (times 2), so this helps ensure tiny/skinny triangles
		// don't have an outsized impact on the final normal per vertex.

		// Accumulate this cross product into each vertex normal slot.
		m_Vertices.at(vertexA).Normal += areaWeightedNormal;
		m_Vertices.at(vertexB).Normal += areaWeightedNormal;
		m_Vertices.at(vertexC).Normal += areaWeightedNormal;
//			vertexNormals[vertexA] += areaWeightedNormal;
//			vertexNormals[vertexB] += areaWeightedNormal;
//			vertexNormals[vertexC] += areaWeightedNormal;
	}

	// Finally, normalize all the sums to get a unit-length, area-weighted average.
	for (int vertex = 0; vertex < m_Vertices.size(); vertex++)
		m_Vertices.at(vertex).Normal = glm::normalize(m_Vertices.at(vertex).Normal);

	SetupBuffers();

}

void Mesh::SetupBuffers()
{
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

