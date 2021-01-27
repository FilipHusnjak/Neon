#include "neopch.h"

#include "Material.h"

namespace Neon
{

	Material::Material(const SharedRef<Shader>& shader)
		: m_Shader(shader)
	{
	}

	void Material::SetProperties(uint32 binding, uint32 index, void* data)
	{
		m_Shader->SetUniformBuffer(binding, index, data);
	}

	void Material::LoadTexture2D(uint32 binding, uint32 index, const std::string& path)
	{
		SharedRef<Texture2D> texture = Texture2D::Create(path, true);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTexture2D(binding, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadDefaultTexture2D(uint32 binding, uint32 index)
	{
		SharedRef<Texture2D> texture = Texture2D::Create();
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load default texture!");
		m_Shader->SetTexture2D(binding, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadTextureCube(uint32 binding, uint32 index, const std::string& path)
	{
		SharedRef<TextureCube> texture = TextureCube::Create(path, true);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTextureCube(binding, index, texture);
		m_Textures.emplace_back(texture);
	}

	void Material::LoadTextureCube(uint32 binding, uint32 index, const std::array<std::string, 6>& paths)
	{
		SharedRef<TextureCube> texture = TextureCube::Create(paths);
		NEO_CORE_ASSERT(texture->Loaded(), "Could not load texture!");
		m_Shader->SetTextureCube(binding, index, texture);
		m_Textures.emplace_back(texture);
	}

} // namespace Neon
