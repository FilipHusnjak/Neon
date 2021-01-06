#pragma once

#include "Neon/Core/Core.h"
#include "PerspectiveCameraController.h"

namespace Neon
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			Vulkan = 1
		};

		struct RenderAPICapabilities
		{
			std::string Vendor;
			std::string Renderer;
			std::string Version;

			int MaxSamples = 0;
			float MaxAnisotropy = 0.0f;
			int MaxTextureUnits = 0;
		};

	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void Render(SharedRef<PerspectiveCameraController>& camera) = 0;
		virtual void Shutdown() = 0;
		virtual void* GetColorImageId() = 0;

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static API Current()
		{
			return s_API;
		}
		static UniqueRef<RendererAPI> Create();

	private:
		static API s_API;
	};
} // namespace Neon
