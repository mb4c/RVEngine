#pragma once


#include <cstdint>
#include "Texture.hpp"

class FrameBuffer
{
public:
	FrameBuffer(uint32_t width, uint32_t height);
	~FrameBuffer();
	void Bind();
	void Unbind();
	void Resize(uint32_t width, uint32_t height);
	uint32_t GetColorTexture() { return texture;}
private:
	uint32_t m_Width;
	uint32_t m_Height;
	unsigned int fbo;
	unsigned int texture;
	unsigned int rbo;
};
