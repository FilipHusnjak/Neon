#pragma once

#include "Neon/Core/Core.h"

namespace Neon
{
	class RendererContext : public RefCounted
	{
	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;

		virtual void BeginFrame() = 0;
		virtual void SwapBuffers() = 0;

		virtual void OnResize(uint32 width, uint32 height) = 0;

		virtual uint32 GetTargetMaxFramesInFlight() const = 0;

		static SharedRef<RendererContext> Create(void* window);
	};
} // namespace Neon
