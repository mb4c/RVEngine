#include "EditorCamera.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <Macros.hpp>

void EditorCamera::SetViewMatrix(glm::mat4 view)
{
	RV_PROFILE_FUNCTION();
	m_ViewMatrix = view;
}
