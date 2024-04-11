#pragma once


#include <cstdint>
#include <Texture2D.hpp>
#include <glm/vec2.hpp>

struct PixelInfo {
	uint EntityID = 0;
	uint DrawID = 0;
	uint PrimID = 0;

	void Print()
	{
		printf("Object %d draw %d prim %d\n", EntityID, DrawID, PrimID);
	}
};

class FrameBuffer
{
public:
	FrameBuffer(uint32_t width, uint32_t height);
	~FrameBuffer();
	void Bind();
	void Unbind();
	void Resize(uint32_t width, uint32_t height);
	uint32_t GetColorTexture() { return texture;}
	uint32_t GetEntityID(glm::vec2 pos);
	uint32_t GetFBO(){return fbo;}
private:
	uint32_t m_Width;
	uint32_t m_Height;
	unsigned int fbo;
	unsigned int texture;
	unsigned int pickingTexture;
	unsigned int rbo;
};
