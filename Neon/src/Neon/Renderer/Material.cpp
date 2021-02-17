#include "neopch.h"

#include "Material.h"

namespace Neon
{
	Material::Material(const SharedRef<Shader>& shader)
		: m_Shader(shader)
	{
	}

	void Material::SetProperties(uint32 index, const MaterialProperties& properties)
	{
		m_Properties = properties;
		m_Shader->SetUniformBuffer("MaterialUBO", index, &properties);
	}

	MaterialProperties& Material::GetProperties()
	{
		return m_Properties;
	}

	void Material::SetTexture2D(const std::string& name, uint32 index, const SharedRef<Texture2D>& texture2D, uint32 mipLevel)
	{
		m_Shader->SetTexture2D(name, index, texture2D, mipLevel);
	}

	void Material::SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& textureCube, uint32 mipLevel)
	{
		m_Shader->SetTextureCube(name, index, textureCube, mipLevel);
	}

	void Material::LoadTexture2D(const std::string& name, uint32 index, const std::string& path,
								 const TextureSpecification& textureSpecification, uint32 mipLevel)
	{
		SharedRef<Texture2D> texture2D = Texture2D::Create(path, textureSpecification);
		NEO_CORE_ASSERT(texture2D->Loaded(), "Could not load texture!");
		SetTexture2D(name, index, texture2D, mipLevel);
	}

	void Material::LoadDefaultTexture2D(const std::string& name, uint32 index, uint32 mipLevel)
	{
		SharedRef<Texture2D> texture2D = Texture2D::Create({});
		NEO_CORE_ASSERT(texture2D->Loaded(), "Could not load default texture!");
		SetTexture2D(name, index, texture2D, mipLevel);
	}

	void Material::LoadTextureCube(const std::string& name, uint32 index, const std::string& path,
								   const TextureSpecification& textureSpecification, uint32 mipLevel)
	{
		SharedRef<TextureCube> textureCube = TextureCube::Create(path, textureSpecification);
		NEO_CORE_ASSERT(textureCube->Loaded(), "Could not load texture!");
		SetTextureCube(name, index, textureCube, mipLevel);
	}

	void Material::LoadTextureCube(const std::string& name, uint32 index, const std::array<std::string, 6>& paths,
								   const TextureSpecification& textureSpecification, uint32 mipLevel)
	{
		SharedRef<TextureCube> textureCube = TextureCube::Create(paths, textureSpecification);
		NEO_CORE_ASSERT(textureCube->Loaded(), "Could not load texture!");
		SetTextureCube(name, index, textureCube, mipLevel);
	}

	SharedRef<Texture2D> Material::GetTexture2D(const std::string& name, uint32 index) const
	{
		return m_Shader->GetTexture2D(name, index);
	}

	SharedRef<TextureCube> Material::GetTextureCube(const std::string& name, uint32 index) const
	{
		return m_Shader->GetTextureCube(name, index);
	}

} // namespace Neon
