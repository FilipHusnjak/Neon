#pragma once

namespace Neon
{
	class Panel : public RefCounted
	{
	public:
		virtual void Render() const = 0;
	};
}
