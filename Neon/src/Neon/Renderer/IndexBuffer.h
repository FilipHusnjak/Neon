#pragma once

namespace Neon
{
	class IndexBuffer : public RefCounted
	{
	public:
		IndexBuffer(uint32 size);
		virtual ~IndexBuffer() = default;

		uint32 GetCount() const
		{
			return m_Size / sizeof(uint32);
		}

		uint32 GetSize() const
		{
			return m_Size;
		}

		static SharedRef<IndexBuffer> Create(void* data, uint32 size);

	protected:
		uint32 m_Size;
	};
} // namespace Neon
