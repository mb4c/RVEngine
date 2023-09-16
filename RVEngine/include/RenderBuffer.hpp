#pragma once


#include <cstdint>

class RenderBuffer
{
public:
	RenderBuffer();
	void Bind();
	void Unbind();
private:
	uint32_t m_RendererID;
};
