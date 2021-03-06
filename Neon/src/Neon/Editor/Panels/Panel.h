#pragma once

namespace Neon
{
	class Panel : public RefCounted
	{
	public:
		virtual ~Panel() = default;

		virtual void Render() const = 0;
	};
} // namespace Neon
