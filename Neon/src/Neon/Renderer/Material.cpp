#include "neopch.h"

#include "Material.h"

namespace Neon
{
	Material::Material(const SharedRef<Shader>& shader)
		: m_Shader(shader)
	{
	}

	void Material::SetProperties(uint32 index, void* data)
	{
		m_Shader->SetUniformBuffer("MaterialUBO", index, data);
	}

	void Material::SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& textureCube)
	{
		m_Shader->SetTextureCube(name, index, textureCube);
		m_Textures.emplace_back(textureCube);
	}

	void Material::LoadTexture2D(const std::string& name, uint32 index, const std::string& path, TextureType type)
	{
		SharedRef<Texture2D> texture = Texture2D::Create(path, type);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTexture2D(name, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadDefaultTexture2D(const std::string& name, uint32 index)
	{
		SharedRef<Texture2D> texture = Texture2D::Create(TextureType::RGB);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load default texture!");
		m_Shader->SetTexture2D(name, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadTextureCube(const std::string& name, uint32 index, const std::string& path, TextureType type)
	{
		SharedRef<TextureCube> textureCube = TextureCube::Create(path, type);
		NEO_CORE_ASSERT(textureCube->Loaded(), "Could not load texture!");
		SetTextureCube(name, index, textureCube);
	}

	void Material::LoadTextureCube(const std::string& name, uint32 index, const std::array<std::string, 6>& paths, TextureType type)
	{
		SharedRef<TextureCube> textureCube = TextureCube::Create(paths, type);
		NEO_CORE_ASSERT(textureCube->Loaded(), "Could not load texture!");
		SetTextureCube(name, index, textureCube);
	}

} // namespace Neon
