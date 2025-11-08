#include "Renderer/Renderer.hpp"

#include <Renderer/Renderer.hpp>
#include "Renderer/EditorCamera.hpp"
#include "Renderer/MaterialUBO.hpp"

std::unique_ptr<Renderer::SceneData> Renderer::s_SceneData = std::make_unique<Renderer::SceneData>();
std::unique_ptr<Renderer::DebugGeometry> Renderer::s_DebugGeometry = std::make_unique<Renderer::DebugGeometry>();

void Renderer::Init()
{
	RV_PROFILE_FUNCTION();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugMessageCallback, 0);

	for (auto& frame : s_SceneData->queryFrames)
	{
		glGenQueries(1, &frame.primitivesQuery);
		glGenQueries(1, &frame.timeElapsedQuery);
	}

	glGenBuffers(1, &s_SceneData->materialUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, s_SceneData->materialUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialUBO), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 9, s_SceneData->materialUBO); // 9 = binding = 9 in GLSL
}

void Renderer::Shutdown()
{
	RV_PROFILE_FUNCTION();

	for (auto& frame : s_SceneData->queryFrames)
	{
		if (frame.primitivesQuery != 0)
			glDeleteQueries(1, &frame.primitivesQuery);
		if (frame.timeElapsedQuery != 0)
			glDeleteQueries(1, &frame.timeElapsedQuery);
	}
}

void Renderer::OnWindowResize(int width, int height)
{
	RV_PROFILE_FUNCTION();

}

void Renderer::BeginScene(EditorCamera &camera)
{
	RV_PROFILE_FUNCTION();
	auto& currentFrame = s_SceneData->queryFrames[s_SceneData->currentQueryFrame];

	if (!currentFrame.active)
	{
		glBeginQuery(GL_PRIMITIVES_GENERATED, currentFrame.primitivesQuery);
		glBeginQuery(GL_TIME_ELAPSED, currentFrame.timeElapsedQuery);
		currentFrame.active = true;
	}

	s_SceneData->ViewProjectionMatrix = camera.GetProjection() * camera.GetViewMatrix();
	s_SceneData->ViewMatrix = camera.GetViewMatrix();
	s_SceneData->ProjectionMatrix = camera.GetProjection();
	RenderStats::GetInstance().DrawCalls = 0;

}

void Renderer::BeginScene(Camera& camera, const glm::mat4& transform)
{
	RV_PROFILE_FUNCTION();

	auto& currentFrame = s_SceneData->queryFrames[s_SceneData->currentQueryFrame];

	if (!currentFrame.active)
	{
		glBeginQuery(GL_PRIMITIVES_GENERATED, currentFrame.primitivesQuery);
		glBeginQuery(GL_TIME_ELAPSED, currentFrame.timeElapsedQuery);
		currentFrame.active = true;
	}

	s_SceneData->ViewProjectionMatrix = camera.GetProjection() * glm::inverse(transform);
	s_SceneData->ViewMatrix = glm::inverse(transform);
	s_SceneData->ProjectionMatrix = camera.GetProjection();
	RenderStats::GetInstance().DrawCalls = 0;

}

void Renderer::EndScene()
{
	RV_PROFILE_FUNCTION();

	auto& currentFrame = s_SceneData->queryFrames[s_SceneData->currentQueryFrame];

	if (currentFrame.active)
	{
		glEndQuery(GL_PRIMITIVES_GENERATED);
		glEndQuery(GL_TIME_ELAPSED);
		currentFrame.active = false;
	}

	s_SceneData->currentQueryFrame = (s_SceneData->currentQueryFrame + 1) % SceneData::QUERY_FRAME_COUNT;

	for (auto& frame : s_SceneData->queryFrames)
	{
		if (!frame.active && frame.primitivesQuery != 0)
		{
			GLint available = 0;
			glGetQueryObjectiv(frame.primitivesQuery, GL_QUERY_RESULT_AVAILABLE, &available);

			if (available)
			{
				glGetQueryObjectuiv(frame.primitivesQuery, GL_QUERY_RESULT, &frame.primitivesResult);
				glGetQueryObjectuiv(frame.timeElapsedQuery, GL_QUERY_RESULT, &frame.timeResult);
			}
		}
	}
}

void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4 &transform, unsigned int entity)
{
	RV_PROFILE_FUNCTION();
	shader->Bind();
	shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
	shader->SetMat4("u_View", s_SceneData->ViewMatrix);
	shader->SetMat4("u_Projection", s_SceneData->ProjectionMatrix);
	shader->SetMat4("u_Transform", transform);
	shader->SetMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(transform))));
	shader->SetUInt("u_ObjectIndex", entity + 1);


	vertexArray->Bind();
	DrawIndexed(vertexArray, 0);
}

void Renderer::SetViewport(int x, int y, int width, int height)
{
	RV_PROFILE_FUNCTION();
	glViewport(x, y, width, height);
}

void Renderer::SetClearColor(const glm::vec4 &color)
{
	RV_PROFILE_FUNCTION();
	glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear()
{
	RV_PROFILE_FUNCTION();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, int indexCount)
{
	RV_PROFILE_FUNCTION();
	vertexArray->Bind();
	uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	RenderStats::GetInstance().DrawCalls++;
}

void Renderer::DrawLines(const std::shared_ptr<VertexArray> &vertexArray, int vertexCount)
{
	RV_PROFILE_FUNCTION();
	vertexArray->Bind();
	glDrawArrays(GL_LINES, 0, vertexCount);
	RenderStats::GetInstance().DrawCalls++;
}

void Renderer::DrawLine(const glm::vec3& start, const glm::vec3& end,const glm::vec4& color, float thickness, bool depthTest)
{
	s_DebugGeometry->AddLine(start, end, color, thickness, depthTest);
}

void Renderer::SetLineWidth(float width)
{
	RV_PROFILE_FUNCTION();
	glLineWidth(width);
}

void Renderer::DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
									const GLchar *message, const void *userParam)
{
	auto const src_str = [source]()
	{
		switch (source)
		{
			case GL_DEBUG_SOURCE_API:
				return "API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				return "WINDOW SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				return "SHADER COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				return "THIRD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION:
				return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER:
				return "OTHER";
			default:
				return "UNKNOWN";
		}
	}();

	auto const type_str = [type]()
	{
		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:
				return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				return "DEPRECATED_BEHAVIOR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				return "UNDEFINED_BEHAVIOR";
			case GL_DEBUG_TYPE_PORTABILITY:
				return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE:
				return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER:
				return "MARKER";
			case GL_DEBUG_TYPE_OTHER:
				return "OTHER";
			default:
				return "UNKNOWN";
		}
	}();

	auto const severity_str = [severity]()
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				return "NOTIFICATION";
			case GL_DEBUG_SEVERITY_LOW:
				return "LOW";
			case GL_DEBUG_SEVERITY_MEDIUM:
				return "MEDIUM";
			case GL_DEBUG_SEVERITY_HIGH:
				return "HIGH";
			default:
				return "UNKNOWN";
		}
	}();
	std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}

GLuint Renderer::GetPrimitivesGenerated()
{
	RV_PROFILE_FUNCTION();

	GLuint maxResult = 0;
	for (const auto& frame : s_SceneData->queryFrames)
	{
		if (frame.primitivesResult > maxResult)
			maxResult = frame.primitivesResult;
	}
	return maxResult;
}

GLuint Renderer::GetTimeElapsed()
{
	RV_PROFILE_FUNCTION();

	GLuint maxResult = 0;
	for (const auto& frame : s_SceneData->queryFrames)
	{
		if (frame.timeResult > maxResult)
			maxResult = frame.timeResult;
	}
	return maxResult;
}

glm::mat4 Renderer::GetView()
{
	return s_SceneData->ViewMatrix;
}

glm::mat4 Renderer::GetViewProjection()
{
	return s_SceneData->ViewProjectionMatrix;
}

Renderer::DebugGeometry& Renderer::GetDebugGeometry()
{
	return *s_DebugGeometry;
}

glm::mat4 Renderer::GetProjection()
{
	return s_SceneData->ProjectionMatrix;
}
