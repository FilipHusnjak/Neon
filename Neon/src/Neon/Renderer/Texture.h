#pragma once

namespace Neon
{
	enum class TextureFormat
	{
		None = 0,

		RGBA8 = 1,
		SRGBA8 = 2,
		RGBA16F = 3,
		RGBA32F = 4,

		Depth = 5
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	enum TextureUsageFlagBits
	{
		ShaderRead = 1 << 0,
		ShaderWrite = 1 << 1,
		InputAttachment = 1 << 2,
		DepthAttachment = 1 << 3,
		ColorAttachment = 1 << 4
	};

	enum class TextureMinMagFilter
	{
		Linear,
		Nearest
	};

	struct TextureSpecification
	{
		uint32 UsageFlags;
		TextureFormat Format = TextureFormat::RGBA8;
		TextureWrap Wrap = TextureWrap::Repeat;
		TextureMinMagFilter MinMagFilter = TextureMinMagFilter::Linear;
		bool Update = true;
		uint32 SampleCount = 1;
		bool UseMipmap = true;
		uint32 Width = 1; // Ignored if path is specified
		uint32 Height = 1; // Ignored if path is specified
	};

	class Texture : public RefCounted
	{
	public:
		static uint32 GetBytesPerPixel(TextureFormat format);
		static uint32 CalculateMaxMipMapCount(uint32 width, uint32 height);

		Texture(const TextureSpecification& specification);
		virtual ~Texture() = default;

		virtual uint32 GetMipLevelCount() const
		{
			return m_MipLevelCount;
		}

		TextureFormat GetFormat() const
		{
			return m_Specification.Format;
		}

		virtual void RegenerateMipMaps() = 0;

	protected:
		TextureSpecification m_Specification{};
		uint32 m_MipLevelCount = 1;
	};

	class Texture2D : public Texture
	{
	public:
		static SharedRef<Texture2D> Create(const TextureSpecification& specification);
		static SharedRef<Texture2D> Create(const std::string& path, const TextureSpecification& specification);

		Texture2D(const TextureSpecification& specification);
		Texture2D(const std::string& path, const TextureSpecification& specification);
		virtual ~Texture2D() = default;

		virtual Buffer GetTextureData() = 0;

		virtual bool Loaded() const = 0;

		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;

		virtual void* GetRendererId() const = 0;

		const std::string& GetPath() const
		{
			return m_Path;
		}

	private:
		const std::string m_Path;
	};

	class TextureCube : public Texture
	{
	public:
		static SharedRef<TextureCube> Create(const TextureSpecification& specification);
		static SharedRef<TextureCube> Create(const std::string& path, const TextureSpecification& specification);
		static SharedRef<TextureCube> Create(const std::array<std::string, 6>& paths, const TextureSpecification& specification);

		TextureCube(const TextureSpecification& specification);
		TextureCube(const std::string& path, const TextureSpecification& specification);
		TextureCube(const std::array<std::string, 6>& paths, const TextureSpecification& specification);
		virtual ~TextureCube() = default;

		virtual Buffer GetTextureData() = 0;

		virtual bool Loaded() const = 0;

		virtual uint32 GetFaceSize() const = 0;

		const std::string& GetPath() const
		{
			return m_Path;
		}

	private:
		const std::string m_Path;
		const std::array<std::string, 6> m_Paths;
	};

} // namespace Neon
