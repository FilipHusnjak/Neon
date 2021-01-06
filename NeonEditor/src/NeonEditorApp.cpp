#include "EditorLayer.h"

#include <Neon/Core/EntryPoint.h>

class SandboxApp : public Neon::Application
{
public:
	SandboxApp()
	{
		PushLayer(new Neon::EditorLayer());
	}

	~SandboxApp() = default;
};

Neon::Application* Neon::CreateApplication()
{
	return new SandboxApp();
}
