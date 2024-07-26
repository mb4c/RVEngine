#pragma once

#include <cstdint>

class BodyUserData
{
public:
	uint32_t entityID = 2137;
	uint32_t otherID = 2137;
	bool isColliding = false;
};