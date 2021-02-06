#pragma once

#include "Neon/Renderer/Pipeline.h"

namespace Neon
{
	class Material : public RefCounted
	{
	public:
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

		void SetProperties(uint32 index, void* data);
		void SetTexture2D(const std::string& name, uint32 index, const SharedRef<Texture2D>& texture2D);
		void SetTextureCube(const std::string& name, uint32 index, const SharedRef<TextureCube>& textureCube);
		void LoadTexture2D(const std::string& name, uint32 index, const std::string& path,
						   const TextureSpecification& textureSpecification);
		void LoadDefaultTexture2D(const std::string& name, uint32 index);
		void LoadTextureCube(const std::string& name, uint32 index, const std::string& path,
							 const TextureSpecification& textureSpecification);
		void LoadTextureCube(const std::string& name, uint32 index, const std::array<std::string, 6>& paths,
							 const TextureSpecification& textureSpecification);

	private:
		SharedRef<Shader> m_Shader;

		std::vector<SharedRef<Texture>> m_Textures;
	};
} // namespace Neon
