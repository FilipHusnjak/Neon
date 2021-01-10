#pragma once

#include "Core.h"

#include <xhash>

namespace Neon
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64 uuid);
		UUID(const UUID& other);

		operator uint64()
		{
			return m_UUID;
		}
		operator const uint64() const
		{
			return m_UUID;
		}

	private:
		uint64 m_UUID;
	};
} // namespace Neon

namespace std
{
	template<>
	struct hash<Neon::UUID>
	{
		std::size_t operator()(const Neon::UUID& uuid) const
		{
			return hash<uint64>()((uint64)uuid);
		}
	};
} // namespace std
