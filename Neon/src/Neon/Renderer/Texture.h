#pragma once

namespace Neon
{
	enum class TextureFormat
	{
		None = 0,
		SRGBA = 1,
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

		Texture() = default;
		Texture(bool srgb);
		virtual ~Texture() = default;

		virtual uint32 GetMipLevelCount() const = 0;

		virtual bool operator==(const Texture& other) const = 0;

	protected:
		bool m_Srgb = false;
		TextureFormat m_Format = TextureFormat::None;
	};

	class Texture2D : public Texture
	{
	public:
		static SharedRef<Texture2D> Create();
		static SharedRef<Texture2D> Create(const std::string& path, bool srgb = false);

		Texture2D() = default;
		Texture2D(const std::string& path, bool srgb);
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
		static SharedRef<TextureCube> Create();
		static SharedRef<TextureCube> Create(const std::string& path, bool srgb = false);
		static SharedRef<TextureCube> Create(const std::array<std::string, 6>& paths, bool srgb = false);

		TextureCube() = default;
		TextureCube(const std::string& path, bool srgb);
		TextureCube(const std::array<std::string, 6>& paths, bool srgb);
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
