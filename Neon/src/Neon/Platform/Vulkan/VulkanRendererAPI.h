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

		void Begin() override;

		void BeginRenderPass(const SharedRef<RenderPass>& renderPass) override;

		void SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform) override;

		void SubmitFullscreenQuad(const SharedRef<Pipeline>& pipeline) override;

		void EndRenderPass() override;
		
		void End() override;

		void WaitIdle() override;

		void Shutdown() override;
	};
} // namespace Neon
