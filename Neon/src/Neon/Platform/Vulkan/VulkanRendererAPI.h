#pragma once

#include "Renderer/RendererAPI.h"
#include "Vulkan.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

namespace Neon
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		~VulkanRendererAPI() override;

		void Init() override;
		void Update(float seconds) override;
		void Render(SharedRef<PerspectiveCameraController>& camera) override;
		void* GetColorImageId() override;

		void Shutdown() override;
	};
} // namespace Neon
