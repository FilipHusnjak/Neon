#pragma once

#include "Neon/Core/Core.h"
#include "Neon/Renderer/Mesh.h"

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

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities capabilities;
			return capabilities;
		}

		static API Current()
		{
			return s_API;
		}

	private:
		static API s_API;
	};
} // namespace Neon
