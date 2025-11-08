#pragma once
#include "glm/vec4.hpp"

struct alignas(16) MaterialUBO
{
    glm::vec4 albedoColor;     // 16 bytes
    glm::vec4 emissionColor;   // 16 bytes
    glm::vec4 parameters;      // roughness, metallic, emissionStrength, flags (as float)
};
