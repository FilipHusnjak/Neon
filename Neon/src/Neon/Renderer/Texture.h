#pragma once

namespace Neon
{
	enum class TextureFormat
	{
		None = 0,
		SRGBA = 1,
		RGBA = 2,
		RGBAFloat32 = 3
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

	class Texture : public RefCounted
	{
	public:
		static uint32 GetBytesPerPixel(TextureFormat format);
		static uint32 CalculateMipMapCount(uint32 width, uint32 height);

		Texture(TextureType type);
		virtual ~Texture() = default;

		virtual uint32 GetMipLevelCount() const = 0;

		TextureFormat GetFormat() const
		{
			return m_Format;
		}

		virtual bool operator==(const Texture& other) const = 0;

	protected:
		TextureType m_Type = TextureType::RGB;
		TextureFormat m_Format = TextureFormat::None;
	};

	class Texture2D : public Texture
	{
	public:
		static SharedRef<Texture2D> Create(TextureType type);
		static SharedRef<Texture2D> Create(const std::string& path, TextureType type);

		Texture2D(TextureType type);
		Texture2D(const std::string& path, TextureType type);
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
		static SharedRef<TextureCube> Create(const uint32 faceSize, TextureType type);
		static SharedRef<TextureCube> Create(const std::string& path, TextureType type);
		static SharedRef<TextureCube> Create(const std::array<std::string, 6>& paths, TextureType type);

		TextureCube(TextureType type);
		TextureCube(const std::string& path, TextureType type);
		TextureCube(const std::array<std::string, 6>& paths, TextureType type);
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
