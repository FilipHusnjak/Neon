#pragma once

namespace Neon
{
	enum class TextureFormat
	{
		None = 0,
		SRGBA = 1,
		RGBA = 2,
		RGBAFloat16 = 3
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	enum class TextureType
	{
		RGB = 0,
		SRGB = 1,
		HDR = 2
	};

	struct TextureSpecification
	{
		TextureType Type = TextureType::RGB;
		uint32 MipLevelCount = 1;
		TextureWrap Wrap = TextureWrap::Repeat;
	};

	class Texture : public RefCounted
	{
	public:
		static uint32 GetBytesPerPixel(TextureFormat format);
		static uint32 CalculateMipMapCount(uint32 width, uint32 height);

		Texture(const TextureSpecification& specification);
		virtual ~Texture() = default;

		virtual uint32 GetMipLevelCount() const = 0;

		TextureFormat GetFormat() const
		{
			return m_Format;
		}

		virtual bool operator==(const Texture& other) const = 0;

	protected:
		TextureSpecification m_Specification;
		TextureFormat m_Format = TextureFormat::None;
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

		const std::string& GetPath() const
		{
			return m_Path;
		}

	private:
		std::string m_Path;
	};

	class TextureCube : public Texture
	{
	public:
		static SharedRef<TextureCube> Create(const uint32 faceSize, const TextureSpecification& specification);
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
