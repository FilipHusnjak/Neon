#pragma once

#include "Neon/Core/Application.h"
#include "Neon/Renderer/Mesh.h"
#include "Neon/Renderer/RenderPass.h"
#include "Neon/Renderer/RendererAPI.h"

namespace Neon
{
	class Renderer
	{
	public:
		static void Init();

		static void BeginRenderPass(const SharedRef<RenderPass>& renderPass);

		static void SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform);

		static void SubmitFullscreenQuad(const SharedRef<GraphicsPipeline>& graphicsPipeline);

		static void DispatchCompute(const SharedRef<ComputePipeline>& computePipeline, uint32 groupCountX, uint32 groupCountY,
									uint32 groupCountZ);

		static void EndRenderPass();

		static void WaitIdle();

		static void* GetFinalImageId();

		static void Shutdown();

		static void EnableWireframe();
		static void DisableWireframe();

		static bool IsWireframeEnabled();

		static void SelectCommandBuffer(const SharedRef<CommandBuffer>& commandBuffer)
		{
			s_SelectedCommandBuffer = commandBuffer;
		}

		static RendererAPI::API GetAPI()
		{
			return RendererAPI::Current();
		}

	private:
		static SharedRef<CommandBuffer> s_SelectedCommandBuffer;
	};
} // namespace Neon
