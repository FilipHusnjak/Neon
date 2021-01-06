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

		virtual void OnAttach()
		{
		}

		virtual void OnDetach()
		{
		}

		virtual void OnUpdate(float ts)
		{
		}

		virtual void OnImGuiRender()
		{
		}

		virtual void OnEvent(Event& event)
		{
		}

		inline const std::string& GetName() const
		{
			return m_DebugName;
		}

	protected:
		std::string m_DebugName;
	};
} // namespace Neon
