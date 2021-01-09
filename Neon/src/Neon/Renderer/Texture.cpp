#include "neopch.h"

#include "Platform/Vulkan/VulkanTexture.h"
#include "RendererAPI.h"
#include "Texture.h"

namespace Neon
{
	uint32 Texture::GetBytesPerPixel(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGB:
				return 3;
			case TextureFormat::RGBA:
				return 4;
		}
		return 0;
	}

	uint32 Texture::CalculateMipMapCount(uint32 width, uint32 height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
		{
			levels++;
		}

		return levels;
	}

	Texture::Texture(TextureFormat format)
		: m_Format(format)
	{
	}

	Texture::Texture(const std::string& path)
		: m_Path(path)
	{
	}

	SharedRef<Texture2D> Texture2D::Create(const std::string& path, bool srgb /*= false*/)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create(path, srgb);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<Texture2D> Texture2D::Create(TextureFormat format, uint32 width, uint32 height,
										   TextureWrap wrap /*= TextureWrap::Clamp*/)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create(format, width, height, wrap);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture2D::Texture2D(const std::string& path)
		: Texture(path)
	{
	}

	Texture2D::Texture2D(TextureFormat format)
		: Texture(format)
	{
	}

} // namespace Neon
