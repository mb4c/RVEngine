#include <Renderer/Model.hpp>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/filesystem.h>

Model::Model(const std::string& path)
{
	RV_PROFILE_FUNCTION();
	LoadModel(path);
	m_Path = path;
}

void Model::SetMaterial(const std::shared_ptr<Material>& material)
{
	for (auto& mesh : *m_Meshes)
	{
		mesh.SetMaterial(material);
	}
}

void Model::LoadModel(const std::string& path)
{
	RV_PROFILE_FUNCTION();
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	m_Path = path;
	Assimp::Importer import;

	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}

	m_Directory = path.substr(0, path.find_last_of('/'));

	std::vector<std::shared_ptr<Material>> materials;

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		RV_PROFILE_SCOPE("Load Materials");
		aiMaterial* material = scene->mMaterials[i];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);

		auto loadedMaterial = std::make_shared<Material>();
		loadedMaterial->materialName = materialName.C_Str();

		unsigned int textureCount = material->GetTextureCount(static_cast<aiTextureType>(aiTextureType_BASE_COLOR));

		for (unsigned int type = aiTextureType_DIFFUSE; type <= aiTextureType_AMBIENT_OCCLUSION; ++type)
		{
			for (int j = 0; j < textureCount; ++j)
			{
				aiString path;
				if (material->GetTexture(static_cast<aiTextureType>(type), j, &path) == AI_SUCCESS)
				{
					if (path.length > 0 && path.data[0] == '*')
					{
						const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());

						if (texture)
						{
							if (texture->mHeight == 0)
							{
								OIIO::Filesystem::IOMemReader memreader(texture->pcData, texture->mWidth);
								auto in = OIIO::ImageInput::open((texture->CheckFormat("jpg") ? "in.jpg" : "in.png"), nullptr, &memreader);
								auto pixels = std::unique_ptr<unsigned char[]>(
										new unsigned char[in->spec().width * in->spec().height * in->spec().nchannels]);
								in->read_image(0, 0, 0, in->spec().nchannels, OIIO::TypeDesc::UINT8, &pixels[0]);
								const OIIO::ImageSpec& spec = in->spec();

								switch (type)
								{
									case aiTextureType_BASE_COLOR:
									{
										loadedMaterial->albedo = std::make_shared<Texture2D>(spec.width, spec.height,
																						 spec.nchannels, pixels.get());
										break;
									}
									case aiTextureType_NORMALS:
									{
										loadedMaterial->normal = std::make_shared<Texture2D>(spec.width, spec.height,
																						 spec.nchannels, pixels.get());
										break;
									}
									case aiTextureType_METALNESS:
									{
										loadedMaterial->metallic = std::make_shared<Texture2D>(spec.width, spec.height,
																						   spec.nchannels,
																						   pixels.get());
										break;
									}
									case aiTextureType_DIFFUSE_ROUGHNESS:
									{
										loadedMaterial->roughness = std::make_shared<Texture2D>(spec.width, spec.height,
																							spec.nchannels,
																							pixels.get());
										break;
									}
									case aiTextureType_AMBIENT_OCCLUSION:
									{
										loadedMaterial->occlusion = std::make_shared<Texture2D>(spec.width, spec.height,
																							spec.nchannels,
																							pixels.get());
										break;
									}
									case aiTextureType_EMISSIVE:
									{
										loadedMaterial->emission = std::make_shared<Texture2D>(spec.width, spec.height,
																							spec.nchannels,
																							pixels.get());
										break;
									}
								}
								in->close();
							}
						}
					}
				}
			}
		}

		if (loadedMaterial->albedo == nullptr)
		{
			loadedMaterial->albedo = std::make_shared<Texture2D>(Texture2D(256, 256, {1, 0, 1, 1}));
		}
		if (loadedMaterial->normal == nullptr)
		{
			loadedMaterial->normal = std::make_shared<Texture2D>(Texture2D(256, 256, {0.5, 0.5, 1, 1}));
		}
		if (loadedMaterial->roughness == nullptr)
		{
			loadedMaterial->roughness = std::make_shared<Texture2D>(Texture2D(256, 256, {1, 1, 1, 1}));
		}
		if (loadedMaterial->metallic == nullptr)
		{
			loadedMaterial->metallic = std::make_shared<Texture2D>(Texture2D(256, 256, {0, 0, 0, 0}));
		}
		if (loadedMaterial->occlusion == nullptr)
		{
			loadedMaterial->occlusion = std::make_shared<Texture2D>(Texture2D(256, 256, {1, 1, 1, 1}));
		}
		if (loadedMaterial->emission == nullptr)
		{
			loadedMaterial->emission = std::make_shared<Texture2D>(Texture2D(256, 256, {0, 0, 0, 0}));
		}

		materials.push_back(loadedMaterial);
	}

	ProcessNode(scene->mRootNode, scene);

	for (auto& mesh : *m_Meshes)
	{
		if (mesh.GetMaterialIndex() < materials.size())
		{
			mesh.SetMaterial(materials[mesh.GetMaterialIndex()]);
		}
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Loading model " << path << " took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
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
	std::vector<Texture2D> textures;

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
		else
		{
			std::cout << m_Path << " HasTangentsAndBitangents() == false" <<  std::endl;
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

	Mesh resultMesh(vertices, indices, textures);
	resultMesh.SetMaterialIndex(mesh->mMaterialIndex);

	return resultMesh;
}
