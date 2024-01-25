#pragma once

#include <glm/vec4.hpp>
#include <memory>
#include <Shader.hpp>
#include <VertexArray.hpp>
#include <Macros.hpp>
#include <RenderStats.hpp>
#include <Camera.hpp>
#include "EditorCamera.hpp"

class Renderer
{
public:
	static void Init();
	static void Shutdown();

	static void OnWindowResize(int width, int height);

	static void BeginScene(EditorCamera& camera);
	static void BeginScene(Camera& camera, const glm::mat4& transform);
	static void EndScene();

	static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f), unsigned int entity = 0);

	static void SetViewport(int x, int y, int width, int height);
	static void SetClearColor(const glm::vec4& color);
	static void Clear();
	static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, int indexCount);
	static void DrawLines(const std::shared_ptr<VertexArray>& vertexArray, int indexCount);
	static void SetLineWidth(float width);
	static void DebugMessageCallback(GLenum source,
									 GLenum type,
									 GLuint id,
									 GLenum severity,
									 GLsizei length,
									 const GLchar* message,
									 const void* userParam);

	static GLuint GetPrimitivesGenerated();
	static GLuint GetTimeElapsed();

private:
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
		GLuint PrimitivesQuery;
		GLuint TimeElapsedQuery;
	};

	static std::unique_ptr<SceneData> s_SceneData;
};
