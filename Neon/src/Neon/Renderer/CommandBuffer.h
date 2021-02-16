#pragma once

#include "Neon/Renderer/IndexBuffer.h"
#include "Neon/Renderer/Pipeline.h"

namespace Neon
{
	enum class CommandBufferType
	{
		Graphics,
		Compute,
		Transfer
	};

	class CommandPool : public RefCounted
	{
	public:
		static SharedRef<CommandPool> Create(CommandBufferType type);

	public:
		CommandPool(CommandBufferType type);
		virtual ~CommandPool() = default;

		CommandBufferType GetType() const
		{
			return m_Type;
		}

		virtual void* GetHandle() const = 0;

	protected:
		CommandBufferType m_Type;
	};

	class StaleResourceWrapper
	{
	public:
		template<typename ResourceType>
		static StaleResourceWrapper Create(const ResourceType& resource)
		{
			class SpecificStaleResource final : public StaleResourceBase
			{
			public:
				SpecificStaleResource(const ResourceType& resource)
					: m_Resource(resource)
				{
				}

			private:
				ResourceType m_Resource;
			};

			return StaleResourceWrapper{SharedRef<StaleResourceBase>(new SpecificStaleResource{resource})};
		}

	public:
		~StaleResourceWrapper() = default;

	private:
		class StaleResourceBase : public RefCounted
		{
		public:
			virtual ~StaleResourceBase() = default;
		};

		StaleResourceWrapper(const SharedRef<StaleResourceBase>& staleResource)
			: m_StaleResource(staleResource)
		{
		}

		SharedRef<StaleResourceBase> m_StaleResource;
	};

	class CommandBuffer : public RefCounted
	{
	public:
		static SharedRef<CommandBuffer> Create(const SharedRef<CommandPool>& commandPool);

	public:
		CommandBuffer(const SharedRef<CommandPool>& commandPool);
		virtual ~CommandBuffer() = default;

		virtual void Begin() const = 0;
		virtual void End() const = 0;
		virtual void Submit() = 0;

		virtual void BeginRenderPass(const SharedRef<RenderPass>& renderPass) const = 0;
		virtual void EndRenderPass() const = 0;
		virtual void BindVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer) const = 0;
		virtual void BindIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer) const = 0;
		virtual void BindPipeline(const SharedRef<Pipeline>& pipeline) const = 0;
		virtual void DrawIndexed(uint32 indexCount, uint32 instanceCount, uint32 firstIndex, int32 vertexOffset,
								 uint32 firstInstance) const = 0;
		virtual void Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) const = 0;

		CommandBufferType GetType() const
		{
			return m_Pool->GetType();
		}

		virtual void* GetHandle() const = 0;

		void SafeDestroyResource(const StaleResourceWrapper& staleResourceWrapper)
		{
			m_ReleaseQueue.push_back(staleResourceWrapper);
		}

		void Flush()
		{
			m_ReleaseQueue.clear();
		}

	protected:
		SharedRef<CommandPool> m_Pool;

		std::vector<StaleResourceWrapper> m_ReleaseQueue;
	};
} // namespace Neon
