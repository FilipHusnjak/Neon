#pragma once

#include "Neon/Gui/GuiContext.h"
#include "Neon/Platform/Vulkan/Vulkan.h"

namespace Neon
{
	class VulkanGuiContext : public GuiContext
	{
	public:
		VulkanGuiContext() = default;

		void Init() override;
		void Shutdown() override;

		void Begin() override;
		void End() override;

	private:
		float m_Time = 0.0f;

		vk::UniqueDescriptorPool m_ImGuiDescPool;
	};
} // namespace Neon
