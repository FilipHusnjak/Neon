#pragma once

#include "Neon/Editor/Panels/Panel.h"
#include "Neon/Renderer/Texture.h"
#include "Neon/Renderer/Mesh.h"

namespace Neon
{
	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel();

		void Render() const override;

	private:
		void RenderMeshProperties(SharedRef<Mesh> mesh) const;

	private:
		SharedRef<Texture2D> m_CheckerboardTex;
	};
} // namespace Neon
