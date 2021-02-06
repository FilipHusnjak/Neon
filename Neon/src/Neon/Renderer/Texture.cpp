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
			case TextureFormat::RGBAFloat16:
				return 8;
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

	Texture::Texture(const TextureSpecification& specification)
		: m_Specification(specification)
	{
	}

	SharedRef<Texture2D> Texture2D::Create(const TextureSpecification& specification)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create(specification);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<Texture2D> Texture2D::Create(const std::string& path, const TextureSpecification& specification)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTexture2D>::Create(path, specification);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Texture2D::Texture2D(const TextureSpecification& specification)
		: Texture(specification)
	{
	}

	Texture2D::Texture2D(const std::string& path, const TextureSpecification& specification)
		: Texture(specification)
		, m_Path(path)
	{
	}

	SharedRef<TextureCube> TextureCube::Create(const uint32 faceSize, const TextureSpecification& specification)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(faceSize, specification);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<TextureCube> TextureCube::Create(const std::string& path, const TextureSpecification& specification)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(path, specification);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	SharedRef<TextureCube> TextureCube::Create(const std::array<std::string, 6>& paths, const TextureSpecification& specification)
	{
		switch (RendererAPI::Current())
		{
			case RendererAPI::API::None:
				return nullptr;
			case RendererAPI::API::Vulkan:
				return SharedRef<VulkanTextureCube>::Create(paths, specification);
		}
		NEO_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	TextureCube::TextureCube(const TextureSpecification& specification)
		: Texture(specification)
	{
	}

	TextureCube::TextureCube(const std::string& path, const TextureSpecification& specification)
		: Texture(specification)
		, m_Path(path)
	{
	}

	TextureCube::TextureCube(const std::array<std::string, 6>& paths, const TextureSpecification& specification)
		: Texture(specification)
		, m_Paths(paths)
	{
	}

} // namespace Neon
