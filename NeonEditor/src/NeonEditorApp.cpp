#include "EditorLayer.h"

#include <Neon/Core/EntryPoint.h>

class NeonEditorApp : public Neon::Application
{
public:
	NeonEditorApp()
	{
		PushLayer(new Neon::EditorLayer());
	}

	~NeonEditorApp() = default;
};

Neon::Application* Neon::CreateApplication()
{
	return new NeonEditorApp();
}
