#pragma once
#include "glm/glm.hpp"

struct alignas(16) CameraUBO
{
    glm::mat4 ViewProjection;
    glm::mat4 View;
    glm::mat4 Projection;
};