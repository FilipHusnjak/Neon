#pragma once

#include "Event/Event.h"

namespace Neon
{
	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer")
			: m_DebugName(std::move(name))
		{
		}
		virtual ~Layer() = default;

		virtual void OnAttach() = 0;

		virtual void OnDetach() = 0;

		virtual void Tick(float deltaSeconds) = 0;

		virtual void OnRenderGui() = 0;

		virtual void OnEvent(Event& event) = 0;

		inline const std::string& GetName() const
		{
			return m_DebugName;
		}

	protected:
		std::string m_DebugName;
	};
} // namespace Neon
