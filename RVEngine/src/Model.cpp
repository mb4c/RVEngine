#include "Model.hpp"

Model::Model(const std::string& path)
{
	RV_PROFILE_FUNCTION();
	LoadModel(path);
}

//void Model::Draw(Shader& shader)
//{
//
//}

void Model::LoadModel(const std::string& path)
{
	RV_PROFILE_FUNCTION();
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path,  aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	m_Directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	RV_PROFILE_FUNCTION();
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes->push_back(ProcessMesh(mesh, scene));
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	RV_PROFILE_FUNCTION();
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};
		// process vertex positions, normals and texture coordinates
		glm::vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		if(mesh->HasTangentsAndBitangents())
		{
			vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
			vertex.Bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
		}

		vertices.push_back(vertex);
	}
	// process indices
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}



//	uint32_t index_count = indices.size();
//
//	for (uint32_t i = 0; i < index_count; i += 3)
//	{
//		uint32_t  i0 = indices[i + 0];
//		uint32_t  i1 = indices[i + 1];
//		uint32_t  i2 = indices[i + 2];
//
//		glm::vec3 edge1 = vertices[i1].Position - vertices[i0].Position;
//		glm::vec3 edge2 = vertices[i2].Position - vertices[i0].Position;
//
//		float deltaU1 = vertices[i1].TexCoords.x - vertices[i0].TexCoords.x;
//		float deltaV1 = vertices[i1].TexCoords.y - vertices[i0].TexCoords.y;
//
//		float deltaU2 = vertices[i2].TexCoords.x - vertices[i0].TexCoords.x;
//		float deltaV2 = vertices[i2].TexCoords.y - vertices[i0].TexCoords.y;
//
//		float dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
//		float fc = 1.0f / dividend;
//
//		glm::vec3 tangent = (glm::vec3)	{(fc * (deltaV2 * edge1.x - deltaV1 * edge2.x)),
//										(fc * (deltaV2 * edge1.y - deltaV1 * edge2.y)),
//										(fc * (deltaV2 * edge1.z - deltaV1 * edge2.z))};
//
//		tangent = glm::normalize(tangent);
//
//		float sx = deltaU1, sy = deltaU2;
//		float tx = deltaV1, ty = deltaV2;
//		float handedness = ((tx * sy - ty * sx) < 0.0f) ? -1.0f : 1.0f;
//
//		glm::vec3 t4 = tangent * handedness;
//		vertices[i0].Tangent = t4;
//		vertices[i1].Tangent = t4;
//		vertices[i2].Tangent = t4;
//	}





	// process material
//	if(mesh->mMaterialIndex >= 0)
//	{
//		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//		std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SHININESS, "texture_roughness");
//		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//		std::vector<Texture> metallicMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_metalness");
//		textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
//	}

	return {vertices, indices, textures};

}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	RV_PROFILE_FUNCTION();
	std::vector<Texture> textures;
	for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for(unsigned int j = 0; j < m_TexturesLoaded.size(); j++)
		{
			if(std::strcmp(m_TexturesLoaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(m_TexturesLoaded[j]);
				skip = true;
				break;
			}
		}
		if(!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = Texture::TextureFromFile(str.C_Str(), m_Directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			m_TexturesLoaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}
