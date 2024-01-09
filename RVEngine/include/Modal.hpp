#pragma once
#include <string>
#include <imgui.h>

class Modal
{
public:
	Modal(const std::string& name)
	{
		m_Name = name;
	};
	void Open()
	{
		m_ShouldOpen = true;
	}

	void Close()
	{
		ImGui::CloseCurrentPopup();
		m_IsOpen = false;
	}

	void Render()
	{
		if (m_ShouldOpen)
		{
			ImGui::OpenPopup(m_Name.c_str());
			m_ShouldOpen = false;
		}
	}

	bool m_ShouldOpen = false;
	bool m_IsOpen = false;
	std::string m_Name;
};
