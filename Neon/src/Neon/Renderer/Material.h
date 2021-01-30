#pragma once

#include "Neon/Renderer/Pipeline.h"
#include "Neon/Renderer/Shader.h"

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
		void LoadTexture2D(const std::string& name, uint32 index, const std::string& path);
		void LoadDefaultTexture2D(const std::string& name, uint32 index);
		void LoadTextureCube(const std::string& name, uint32 index, const std::string& path);
		void LoadTextureCube(const std::string& name, uint32 index, const std::array<std::string, 6>& paths);

	private:
		SharedRef<Shader> m_Shader;

		std::vector<SharedRef<Texture>> m_Textures;
	};
} // namespace Neon
