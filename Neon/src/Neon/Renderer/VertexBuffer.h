#pragma once

namespace Neon
{
	enum class ShaderDataType
	{
		None = 0,
		Int,
		Int2,
		Int3,
		Int4,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Bool
	};

	static uint32 ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:
				return 4;
			case ShaderDataType::Float2:
				return 4 * 2;
			case ShaderDataType::Float3:
				return 4 * 3;
			case ShaderDataType::Float4:
				return 4 * 4;
			case ShaderDataType::Mat3:
				return 4 * 3 * 3;
			case ShaderDataType::Mat4:
				return 4 * 4 * 4;
			case ShaderDataType::Int:
				return 4;
			case ShaderDataType::Int2:
				return 4 * 2;
			case ShaderDataType::Int3:
				return 4 * 3;
			case ShaderDataType::Int4:
				return 4 * 4;
			case ShaderDataType::Bool:
				return 1;
		}

		NEO_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct VertexBufferElement
	{
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;

		VertexBufferElement() = default;

		VertexBufferElement(ShaderDataType type)
			: Type(type)
			, Size(ShaderDataTypeSize(type))
			, Offset(0)
		{
		}
	};

	class VertexBufferLayout
	{
	public:
		VertexBufferLayout() = default;

		VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
			: m_Elements(elements)
		{
			uint32 offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

		uint32 GetStride() const
		{
			return m_Stride;
		}
		const std::vector<VertexBufferElement>& GetElements() const
		{
			return m_Elements;
		}
		uint32 GetElementCount() const
		{
			return static_cast<uint32>(m_Elements.size());
		}

		std::vector<VertexBufferElement>::iterator begin()
		{
			return m_Elements.begin();
		}
		std::vector<VertexBufferElement>::iterator end()
		{
			return m_Elements.end();
		}
		std::vector<VertexBufferElement>::const_iterator begin() const
		{
			return m_Elements.begin();
		}
		std::vector<VertexBufferElement>::const_iterator end() const
		{
			return m_Elements.end();
		}

	private:
		std::vector<VertexBufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer : public RefCounted
	{
	public:
		VertexBuffer(uint32 size, const VertexBufferLayout& layout);
		virtual ~VertexBuffer() = default;

		uint32 GetSize() const
		{
			return m_Size;
		}

		const VertexBufferLayout& GetLayout() const
		{
			return m_Layout;
		}

		static SharedRef<VertexBuffer> Create(void* data, uint32 size, const VertexBufferLayout& layout);

	protected:
		uint32 m_Size;
		VertexBufferLayout m_Layout;
	};
} // namespace Neon
