#pragma once

namespace Neon
{
	class GuiContext : public RefCounted
	{
	public:
		static SharedRef<GuiContext> Create();

	public:
		GuiContext() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;
	};
} // namespace Neon
