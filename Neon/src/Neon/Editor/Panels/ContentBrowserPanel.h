#pragma once

#include "Neon/Editor/Panels/Panel.h"

namespace Neon
{
	class ContentBrowserPanel : public Panel
	{
	public:
		~ContentBrowserPanel() = default;

		void Render() const override;
	};
} // namespace Neon
