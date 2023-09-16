#pragma once


#include <cstdint>
#include <memory>

class IndexBuffer
{
public:
	IndexBuffer(uint32_t* indices, uint32_t count);
	~IndexBuffer();

	void Bind();
	void Unbind();

	uint32_t GetCount();

	static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);

private:
	uint32_t m_RendererID;
	uint32_t m_Count;
};
