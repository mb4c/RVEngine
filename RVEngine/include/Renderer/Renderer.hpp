#pragma once

#include <glm/vec4.hpp>
#include <memory>
#include <Renderer/Shader.hpp>
#include <Renderer/VertexArray.hpp>
#include <Macros.hpp>
#include <Renderer/RenderStats.hpp>
#include <Renderer/Camera.hpp>
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
	static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.0f), float thickness = 1.0f, bool depthTest = true);

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
	static glm::mat4 GetProjection();
	static glm::mat4 GetView();
	static glm::mat4 GetViewProjection();
	static GLuint GetMaterialUBO() { return s_SceneData->materialUBO; }
	struct DebugGeometry
	{
		struct Line
		{
			glm::vec3 Start;
			glm::vec3 End;
			glm::vec4 Color;
			float Thickness = 1.0f;
			bool DepthTest = true;
		};

		std::vector<Line> Lines;

		void AddLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.0f), const float thickness = 1.0f, const bool depthTest = true)
		{
			Lines.push_back({ start, end, color, thickness, depthTest });
		}

		void Clear()
		{
			Lines.clear();
		}
	};

	static DebugGeometry& GetDebugGeometry();
private:
	struct QueryFrame
	{
		GLuint primitivesQuery = 0;
		GLuint timeElapsedQuery = 0;
		GLuint primitivesResult = 0;
		GLuint timeResult = 0;
		bool active = false;
	};
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;

		static constexpr int QUERY_FRAME_COUNT = 3;
		std::array<QueryFrame, QUERY_FRAME_COUNT> queryFrames;
		int currentQueryFrame = 0;

		GLuint materialUBO = 0;
	};



	static std::unique_ptr<SceneData> s_SceneData;
	static std::unique_ptr<DebugGeometry> s_DebugGeometry;
};
