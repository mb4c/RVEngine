#pragma once
#include <Scene.hpp>
#include <filesystem>


class SceneSerializer
{
public:
	SceneSerializer() = default;
	SceneSerializer(const std::shared_ptr<Scene>& scene);

	void Serialize(const std::filesystem::path& path);
	bool Deserialize(const std::filesystem::path& path);

	void SerializeRuntime(const std::filesystem::path& path);
	void DeserializeRuntime(const std::filesystem::path& path);

	void SetContext(const std::shared_ptr<Scene>& scene);
private:
	std::shared_ptr<Scene> m_Scene;

};
