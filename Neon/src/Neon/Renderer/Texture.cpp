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

	SharedRef<Texture2D> Texture2D::Create()
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create();
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture2D::Texture2D(const std::string& path)
		: Texture(path)
	{
	}

	SharedRef<TextureCube> TextureCube::Create()
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create();
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<TextureCube> TextureCube::Create(const std::string& path, bool srgb /*= false*/)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(path, srgb);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<TextureCube> TextureCube::Create(const std::array<std::string, 6>& paths,
																		bool srgb /*= false*/)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(paths, srgb);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	TextureCube::TextureCube(const std::string& path)
		: Texture(path)
	{
	}

} // namespace Neon
