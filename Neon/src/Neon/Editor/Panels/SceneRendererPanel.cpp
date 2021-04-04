#include "neopch.h"

#include "Neon/Editor/Panels/SceneRendererPanel.h"
#include "Neon/Renderer/Renderer.h"

#include <imgui/imgui.h>

namespace Neon
{
	void SceneRendererPanel::Render() const
	{
		ImGui::Begin("Scene Renderer");

		bool drawWireframe = Renderer::IsWireframeEnabled();
		if (ImGui::Checkbox("DrawWireframe##DrawWireframe", &drawWireframe))
		{
			if (drawWireframe)
			{
				Renderer::EnableWireframe();
			}
			else
			{
				Renderer::DisableWireframe();
			}
		}

		ImGui::End();
	}
} // namespace Neon
