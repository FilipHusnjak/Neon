#pragma once

#include "Neon/Editor/Panels/Panel.h"
#include "Neon/Renderer/Texture.h"
#include "Neon/Renderer/Mesh.h"

namespace Neon
{
	class Actor;

	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel() = default;

		void Render() const override;

	private:
		void DrawActorComponents(SharedRef<Actor> actor) const;	
	};
} // namespace Neon
