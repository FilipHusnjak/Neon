#pragma once

#include "Neon/Core/Layer.h"

namespace Neon
{
class ImGuiLayer : public Layer
{
public:
	virtual void Begin() = 0;
	virtual void End() = 0;

	static ImGuiLayer* Create();
};
} // namespace Neon