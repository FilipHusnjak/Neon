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
			case TextureFormat::RGBA:
				return 4;
			case TextureFormat::SRGBA:
				return 4;
			case TextureFormat::RGBAFloat32:
				return 16;
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

	Texture::Texture(TextureType type)
		: m_Type(type)
	{
	}

	SharedRef<Texture2D> Texture2D::Create(TextureType type)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create(type);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<Texture2D> Texture2D::Create(const std::string& path, TextureType type)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create(path, type);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture2D::Texture2D(TextureType type)
		: Texture(type)
	{
	}

	Texture2D::Texture2D(const std::string& path, TextureType type)
		: Texture(type)
		, m_Path(path)
	{
	}

	SharedRef<TextureCube> TextureCube::Create(const uint32 faceSize, TextureType type)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(faceSize, type);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<TextureCube> TextureCube::Create(const std::string& path, TextureType type)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(path, type);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<TextureCube> TextureCube::Create(const std::array<std::string, 6>& paths, TextureType type)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(paths, type);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	TextureCube::TextureCube(TextureType type)
		: Texture(type)
	{
	}

	TextureCube::TextureCube(const std::string& path, TextureType type)
		: Texture(type)
		, m_Path(path)
	{
	}

	TextureCube::TextureCube(const std::array<std::string, 6>& paths, TextureType type)
		: Texture(type)
		, m_Paths(paths)
	{
	}

} // namespace Neon
