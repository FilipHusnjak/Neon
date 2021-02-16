#pragma once

#include "Neon/Core/Core.h"
#include "Neon/Renderer/CommandBuffer.h"

namespace Neon
{
	class RendererContext : public RefCounted
	{
	public:
		static SharedRef<RendererContext> Create(void* window);

	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;

		virtual void Init() = 0;

		virtual void BeginFrame() = 0;
		virtual void SwapBuffers() = 0;

		virtual void OnResize(uint32 width, uint32 height) = 0;

		virtual uint32 GetTargetMaxFramesInFlight() const = 0;

		virtual SharedRef<CommandBuffer>& GetPrimaryRenderCommandBuffer() = 0;

		virtual void WaitIdle() const = 0;

		virtual SharedRef<CommandBuffer> GetCommandBuffer(CommandBufferType type, bool begin) const = 0;
		virtual void SubmitCommandBuffer(SharedRef<CommandBuffer>& commandBuffer) const = 0;

		void SafeDeleteResource(const StaleResourceWrapper& staleResourceWrapper);

		static SharedRef<RendererContext> Get();
	};
} // namespace Neon
