#pragma once

#include "Neon/Renderer/Pipeline.h"

namespace Neon
{
	struct MaterialProperties
	{
		glm::vec4 AlbedoColor;
		float HasAlbedoTexture;
		float HasNormalTex;
		float Metalness;
		float HasMetalnessTex;
		float Roughness;
		float HasRoughnessTex;
	};

	class Material
	{
	public:
		Material() = default;
		Material(const SharedRef<Shader>& shader);
		~Material() = default;

		SharedRef<Shader> GetShader()
		{
			return m_Shader;
		}
		const SharedRef<Shader>& GetShader() const
		{
			return m_Shader;
		}

		void SetProperties(uint32 index, const MaterialProperties& properties);
		MaterialProperties& GetProperties();

		void SetTexture2D(const std::string& name, uint32 index, const SharedRef<Texture2D>& texture2D, uint32 mipLevel);
		void SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& textureCube, uint32 mipLevel);
		void LoadTexture2D(const std::string& name, uint32 index, const std::string& path,
						   const TextureSpecification& textureSpecification, uint32 mipLevel);
		void LoadDefaultTexture2D(const std::string& name, uint32 index, uint32 mipLevel);
		void LoadTextureCube(const std::string& name, uint32 index, const std::string& path,
							 const TextureSpecification& textureSpecification, uint32 mipLevel);
		void LoadTextureCube(const std::string& name, uint32 index, const std::array<std::string, 6>& paths,
							 const TextureSpecification& textureSpecification, uint32 mipLevel);

		SharedRef<Texture2D> GetTexture2D(const std::string& name, uint32 index) const;
		SharedRef<TextureCube> GetTextureCube(const std::string& name, uint32 index) const;

	private:
		SharedRef<Shader> m_Shader;

		MaterialProperties m_Properties;
	};
} // namespace Neon
