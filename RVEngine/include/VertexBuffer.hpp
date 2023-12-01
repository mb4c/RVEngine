#pragma once


#include <cstdint>
#include <memory>
#include <BufferLayout.hpp>

class VertexBuffer
{
public:
	VertexBuffer(uint32_t size);
	VertexBuffer(float* vertices, uint32_t size);
	~VertexBuffer();

	void Bind();
	void Unbind();

	void SetData(const void* data, uint32_t size);

	const BufferLayout& GetLayout();
	void SetLayout(const BufferLayout& layout);

	static std::shared_ptr<VertexBuffer> Create(uint32_t size);
	static std::shared_ptr<VertexBuffer> Create(float* vertices, uint32_t size);
private:
	uint32_t m_RendererID;
	BufferLayout m_Layout;
};