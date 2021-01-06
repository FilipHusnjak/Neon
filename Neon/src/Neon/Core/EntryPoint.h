#pragma once

#include "neopch.h"

#include "Application.h"

extern Neon::Application* Neon::CreateApplication();

int main(int argc, char** argv)
{
	Neon::InitializeCore();
	auto app = Neon::CreateApplication();
	app->Run();
	delete app;
	Neon::ShutdownCore();
	return 0;
}
