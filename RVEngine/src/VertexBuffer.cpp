#include <glad/glad.h>
#include <VertexBuffer.hpp>
#include <Macros.hpp>

VertexBuffer::VertexBuffer(uint32_t size)
{
	RV_PROFILE_FUNCTION();
	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(float *vertices, uint32_t size)
{
	RV_PROFILE_FUNCTION();
	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{

	RV_PROFILE_FUNCTION();
	glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::Bind()
{
	RV_PROFILE_FUNCTION();
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void VertexBuffer::Unbind()
{
	RV_PROFILE_FUNCTION();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void *data, uint32_t size)
{
	RV_PROFILE_FUNCTION();
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

const BufferLayout &VertexBuffer::GetLayout()
{
	RV_PROFILE_FUNCTION();
	return m_Layout;
}

void VertexBuffer::SetLayout(const BufferLayout &layout)
{
	RV_PROFILE_FUNCTION();
	m_Layout = layout;
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size)
{
	RV_PROFILE_FUNCTION();
	return std::make_shared<VertexBuffer>(size);
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size)
{
	RV_PROFILE_FUNCTION();
	return std::make_shared<VertexBuffer>(vertices, size);

}
