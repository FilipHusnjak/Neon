#pragma once

#include "ImGui/ImGuiLayer.h"
#include "Vulkan.h"

namespace Neon
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		void Begin() override;
		void End() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() override;

	private:
		float m_Time = 0.0f;

		vk::UniqueDescriptorPool m_ImGuiDescPool;
	};
} // namespace Neon
