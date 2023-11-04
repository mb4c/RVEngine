#include "Renderer.hpp"

std::unique_ptr<Renderer::SceneData> Renderer::s_SceneData = std::make_unique<Renderer::SceneData>();

void Renderer::Init()
{
	RV_PROFILE_FUNCTION();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugMessageCallback, 0);
}

void Renderer::Shutdown()
{
	RV_PROFILE_FUNCTION();

}

void Renderer::OnWindowResize(int width, int height)
{
	RV_PROFILE_FUNCTION();

}

void Renderer::BeginScene(PerspectiveCamera &camera)
{
	RV_PROFILE_FUNCTION();
	s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	RenderStats::GetInstance().DrawCalls = 0;

}

void Renderer::EndScene()
{
	RV_PROFILE_FUNCTION();

}

void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4 &transform, unsigned int entity)
{
	RV_PROFILE_FUNCTION();
	shader->Bind();
	shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
	shader->SetMat4("u_Transform", transform);
	shader->SetMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(transform))));
	shader->SetUInt("u_ObjectIndex", entity + 1);

	const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers( 2, buffers );

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

void Renderer::SetLineWidth(float width)
{
	RV_PROFILE_FUNCTION();
	glLineWidth(width);
}

void Renderer::DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
									const GLchar *message, const void *userParam)
{
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			 ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
			 type, severity, message );
}
