#pragma once


#include <cstdint>
#include <Renderer/Texture2D.hpp>
#include <glm/vec2.hpp>

struct PixelInfo {
	uint32_t EntityID = 0;
	uint32_t DrawID = 0;
	uint32_t PrimID = 0;

	void Print()
	{
		printf("Object %d draw %d prim %d\n", EntityID, DrawID, PrimID);
	}
};

struct FrameBufferProperties
{
	uint32_t width;
	uint32_t height;
	uint32_t internalFormat = GL_RGB;
	uint32_t format = GL_RGB;
	uint32_t type = GL_UNSIGNED_BYTE;
	uint32_t filtering = GL_LINEAR;
	uint32_t samples = 1;
};

class FrameBuffer
{
public:
	FrameBuffer(FrameBufferProperties props);
	~FrameBuffer();
	void Bind();
	void Unbind();
	void Resize(uint32_t width, uint32_t height);
	uint32_t GetColorTexture() { return texture;}
	uint32_t GetEntityID(glm::vec2 pos);
	uint32_t GetFBO(){return fbo;}
	void BlitTo(FrameBuffer& target);

private:
	uint32_t m_Width;
	uint32_t m_Height;
	unsigned int fbo;
	unsigned int texture;
	unsigned int pickingTexture;
	unsigned int rbo;
};
