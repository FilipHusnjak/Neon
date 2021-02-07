#include "neopch.h"

#include "Neon/Renderer/SceneRenderer.h"
#include "Renderer.h"

namespace Neon
{
	static SharedRef<VertexBuffer> s_QuadVertexBuffer;
	static SharedRef<IndexBuffer> s_QuadIndexBuffer;

	static const std::vector<glm::vec2> m_QuadVertices = {{-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}};
	static const std::vector<uint32> m_QuadIndices = {0, 2, 1, 1, 2, 3};

	SharedRef<CommandBuffer> Renderer::s_SelectedCommandBuffer;

	void Renderer::Init()
	{
		s_QuadVertexBuffer =
			VertexBuffer::Create(m_QuadVertices.data(), static_cast<uint32>(m_QuadVertices.size()) * sizeof(m_QuadVertices[0]),
								 VertexBufferLayout({ShaderDataType::Float2}));
		s_QuadIndexBuffer =
			IndexBuffer::Create(m_QuadIndices.data(), static_cast<uint32>(m_QuadIndices.size()) * sizeof(m_QuadIndices[0]));

		SceneRenderer::Init();
	}

	void Renderer::BeginRenderPass(const SharedRef<RenderPass>& renderPass)
	{
		NEO_CORE_ASSERT(s_SelectedCommandBuffer);

		s_SelectedCommandBuffer->BeginRenderPass(renderPass);
	}

	void Renderer::SubmitMesh(const SharedRef<Mesh>& mesh, const glm::mat4& transform)
	{
		NEO_CORE_ASSERT(s_SelectedCommandBuffer);

		s_SelectedCommandBuffer->BindPipeline(mesh->GetGraphicsPipeline());
		s_SelectedCommandBuffer->BindVertexBuffer(mesh->GetVertexBuffer());
		s_SelectedCommandBuffer->BindIndexBuffer(mesh->GetIndexBuffer());

		const auto& submeshes = mesh->GetSubmeshes();
		for (const auto& submesh : submeshes)
		{
			s_SelectedCommandBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.BaseIndex, submesh.BaseVertex, 0);
		}
	}

	void Renderer::SubmitFullscreenQuad(const SharedRef<GraphicsPipeline>& graphicsPipeline)
	{
		NEO_CORE_ASSERT(s_SelectedCommandBuffer);

		s_SelectedCommandBuffer->BindPipeline(graphicsPipeline);
		s_SelectedCommandBuffer->BindVertexBuffer(s_QuadVertexBuffer);
		s_SelectedCommandBuffer->BindIndexBuffer(s_QuadIndexBuffer);
		s_SelectedCommandBuffer->DrawIndexed(6, 1, 0, 0, 0);
	}

	void Renderer::DispatchCompute(const SharedRef<ComputePipeline>& computePipeline, uint32 groupCountX, uint32 groupCountY,
								   uint32 groupCountZ)
	{
		auto commandBuffer = RendererContext::Get()->GetCommandBuffer(CommandBufferType::Compute, true);
		commandBuffer->BindPipeline(computePipeline);
		commandBuffer->Dispatch(groupCountX, groupCountY, groupCountZ);
		RendererContext::Get()->SubmitCommandBuffer(commandBuffer);
	}

	void Renderer::EndRenderPass()
	{
		NEO_CORE_ASSERT(s_SelectedCommandBuffer);

		s_SelectedCommandBuffer->EndRenderPass();
	}

	void Renderer::WaitIdle()
	{
		RendererContext::Get()->WaitIdle();
	}

	void* Renderer::GetFinalImageId()
	{
		return SceneRenderer::GetFinalImageId();
	}

	void Renderer::Shutdown()
	{
		WaitIdle();

		s_SelectedCommandBuffer.Reset();
		s_QuadVertexBuffer.Reset();
		s_QuadIndexBuffer.Reset();

		SceneRenderer::Shutdown();
	}
} // namespace Neon
