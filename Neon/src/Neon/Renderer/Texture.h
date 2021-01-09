#pragma once

namespace Neon
{
	enum class TextureFormat
	{
		None = 0,
		RGB = 1,
		RGBA = 2,
		Float16 = 3
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp = 1,
		Repeat = 2
	};

	class Texture : public RefCounted
	{
	public:
		static uint32 GetBytesPerPixel(TextureFormat format);
		static uint32 CalculateMipMapCount(uint32 width, uint32 height);

		Texture(TextureFormat format);
		Texture(const std::string& path);
		virtual ~Texture() = default;

		TextureFormat GetFormat() const
		{
			return m_Format;
		}

		const std::string& GetPath() const
		{
			return m_Path;
		}

		virtual uint32 GetMipLevelCount() const = 0;

		virtual bool operator==(const Texture& other) const = 0;

	private:
		TextureFormat m_Format;
		std::string m_Path;
	};

	class Texture2D : public Texture
	{
	public:
		static SharedRef<Texture2D> Create(TextureFormat format, uint32 width, uint32 height,
										   TextureWrap wrap = TextureWrap::Clamp);
		static SharedRef<Texture2D> Create(const std::string& path, bool srgb = false);

		Texture2D(TextureFormat format);
		Texture2D(const std::string& path);
		virtual ~Texture2D() = default;

		virtual void Resize(uint32 width, uint32 height) = 0;
		virtual Buffer GetTextureData() = 0;

		virtual bool Loaded() const = 0;

		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;
	};

} // namespace Neon
