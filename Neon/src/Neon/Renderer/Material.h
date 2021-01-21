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

		const SharedRef<Shader>& GetShader() const
		{
			return m_Shader;
		}

		void SetProperties(uint32 binding, uint32 index, void* data);
		void LoadTexture2D(uint32 binding, uint32 index, const std::string& path);
		void LoadTextureCube(uint32 binding, uint32 index, const std::string& path);

	private:
		SharedRef<Shader> m_Shader;

		std::vector<SharedRef<Texture>> m_Textures;
	};
} // namespace Neon
