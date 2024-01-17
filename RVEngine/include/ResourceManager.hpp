#pragma once

#include <Singleton.hpp>
#include <Shader.hpp>
#include <unordered_map>
#include <Model.hpp>
#include <Texture2D.hpp>

class ResourceManager : public Singleton<ResourceManager>
{
public:
	ResourceManager(token)
	{
		m_Models.emplace("plane", std::make_shared<Model>("res/plane.fbx"));
		m_Models.emplace("sphere05", std::make_shared<Model>("res/sphere05.fbx"));
		m_Models.emplace("cube", std::make_shared<Model>("res/cube.fbx"));

		m_Shaders.emplace("pbr", std::make_shared<Shader>("res/shaders/PBR_vert.glsl", "res/shaders/PBR_frag.glsl"));
		m_Shaders.emplace("flat", std::make_shared<Shader>("res/shaders/FlatColor.vert", "res/shaders/FlatColor.frag"));
		m_Shaders.emplace("sprite", std::make_shared<Shader>("res/shaders/Sprite.vert", "res/shaders/Sprite.frag"));
		m_Shaders.emplace("cubemap", std::make_shared<Shader>("res/shaders/Cubemap_vert.glsl", "res/shaders/Cubemap_frag.glsl"));
		m_Shaders.emplace("convolution", std::make_shared<Shader>("res/shaders/Cubemap_vert.glsl", "res/shaders/Convolution_frag.glsl"));
		m_Shaders.emplace("skybox", std::make_shared<Shader>("res/shaders/Skybox_vert.glsl", "res/shaders/Skybox_frag.glsl"));
		m_Shaders.emplace("prefilter", std::make_shared<Shader>("res/shaders/Prefilter_vert.glsl", "res/shaders/Prefilter_frag.glsl"));
		m_Shaders.emplace("brdf", std::make_shared<Shader>("res/shaders/brdf_vert.glsl", "res/shaders/brdf_frag.glsl"));


		m_Textures.emplace("brickwall_albedo", std::make_shared<Texture2D>(Texture2D("res/brickwall.jpg")));
		m_Textures.emplace("brickwall_normal", std::make_shared<Texture2D>(Texture2D("res/brickwall_normal.jpg")));
		m_Textures.emplace("brickwall_orm", std::make_shared<Texture2D>(Texture2D("res/brickwall_ORM.png")));


		// Default textures
		m_Textures.emplace("default_normal", std::make_shared<Texture2D>(Texture2D(256, 256, {0.5, 0.5, 1, 1})));
		m_Textures.emplace("default_albedo", std::make_shared<Texture2D>(Texture2D(256, 256, {1, 1, 1, 1})));
		m_Textures.emplace("default_missing", std::make_shared<Texture2D>(Texture2D("res/missing.png")));

		//TODO: temp
		for (auto& it: m_Textures)
		{
			it.second->SetName(it.first);
		}



		auto brickwallMat = std::make_shared<Material>();
		brickwallMat->albedo = GetTexture("brickwall_albedo");
		brickwallMat->normal= GetTexture("brickwall_normal");
		brickwallMat->occlusionRoughnessMetallic = GetTexture("brickwall_orm");
		m_Materials.emplace("brickwall", brickwallMat);

		auto defaultMat = std::make_shared<Material>();
		defaultMat->albedo = GetTexture("default_albedo");
		defaultMat->normal= GetTexture("default_normal");
		defaultMat->occlusionRoughnessMetallic = GetTexture("brickwall_orm");
		m_Materials.emplace("default_pbr", defaultMat);


		// icons
		m_Textures.emplace("icon_folder", std::make_shared<Texture2D>(Texture2D("res/icons/folder.png")));
		m_Textures.emplace("icon_file", std::make_shared<Texture2D>(Texture2D("res/icons/file.png")));


	};

	template<class T>
	T Get(const std::string& name, std::unordered_map<std::string, T> map)
	{
		if (auto it = map.find(name); it != map.end())
		{
			return it->second;
		} else
		{
			return map.at("default_missing");
		}
	}

	std::shared_ptr<Model> GetModel(const std::string& name)
	{
		return m_Models.at(name);
	}

	std::shared_ptr<Shader> GetShader(const std::string& name)
	{
		return m_Shaders.at(name);
	}

	std::shared_ptr<Texture2D> GetTexture(const std::string& name)
	{
		return Get<std::shared_ptr<Texture2D>>(name, m_Textures);
	}

	std::shared_ptr<Material> GetMaterial(const std::string& name)
	{
		return m_Materials.at(name);
	}

	void AddModel(const std::string& name, const std::shared_ptr<Model>& model)
	{
		m_Models.emplace(name, model);
	}
	void AddShader(const std::string& name, const std::shared_ptr<Shader>& shader)
	{
		m_Shaders.emplace(name, shader);
	}

	void AddTexture(const std::string& name, const std::shared_ptr<Texture2D>& texture)
	{
		m_Textures.emplace(name, texture);
	}

	void AddMaterial(const std::string& name, const std::shared_ptr<Material>& material)
	{
		m_Materials.emplace(name, material);
	}

	std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	std::unordered_map<std::string, std::shared_ptr<Model>> m_Models;
	std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;
	std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
};
