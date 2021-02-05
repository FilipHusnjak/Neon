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

	void Material::LoadTexture2D(const std::string& name, uint32 index, const std::string& path, bool srgb)
	{
		SharedRef<Texture2D> texture = Texture2D::Create(path, srgb);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTexture2D(name, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadDefaultTexture2D(const std::string& name, uint32 index)
	{
		SharedRef<Texture2D> texture = Texture2D::Create();
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load default texture!");
		m_Shader->SetTexture2D(name, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadTextureCube(const std::string& name, uint32 index, const std::string& path, bool srgb)
	{
		SharedRef<TextureCube> texture = TextureCube::Create(path, srgb);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTextureCube(name, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadTextureCube(const std::string& name, uint32 index, const std::array<std::string, 6>& paths, bool srgb)
	{
		SharedRef<TextureCube> texture = TextureCube::Create(paths, srgb);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTextureCube(name, index, texture);
		m_Textures.emplace_back(texture);
	}

} // namespace Neon
