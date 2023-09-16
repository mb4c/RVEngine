#include "PerspectiveCamera.hpp"
#include "Macros.hpp"

PerspectiveCamera::PerspectiveCamera()
{
	RV_PROFILE_FUNCTION();
	m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

void PerspectiveCamera::RecalculateViewMatrix()
{
	RV_PROFILE_FUNCTION();
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
						  glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

	m_ViewMatrix = glm::inverse(transform);
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void PerspectiveCamera::SetViewMatrix(glm::mat4 view)
{
	RV_PROFILE_FUNCTION();
	m_ViewMatrix = view;
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

}

void PerspectiveCamera::Resize(float width, float height)
{
	m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
}
