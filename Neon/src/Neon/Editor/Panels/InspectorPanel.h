#pragma once

#include "Neon/Editor/Panels/Panel.h"
#include "Neon/Renderer/Texture.h"

namespace Neon
{
	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel();

		void Render() const override;

	private:
		SharedRef<Texture2D> m_CheckerboardTex;
	};
} // namespace Neon
