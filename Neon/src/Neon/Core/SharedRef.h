#pragma once

// Intrinsic ref counting system
namespace Neon
{
	class RefCounted
	{
	public:
		void AddRef() const
		{
			m_RefCount++;
		}
		void RemoveRef() const
		{
			m_RefCount--;
		}

		uint32 GetRefCount() const
		{
			return m_RefCount;
		}

	private:
		mutable uint32 m_RefCount = 0;
	};

	template<typename T>
	class SharedRef
	{
	public:
		SharedRef()
			: m_Ptr(nullptr)
		{
		}

		SharedRef(std::nullptr_t n)
			: m_Ptr(nullptr)
		{
		}

		SharedRef(T* ptr)
			: m_Ptr(ptr)
		{
			static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");
			AddRef();
		}

		template<typename T2>
		SharedRef(const SharedRef<T2>& other)
		{
			m_Ptr = (T*)other.m_Ptr;
			AddRef();
		}

		template<typename T2>
		SharedRef(SharedRef<T2>&& other)
		{
			m_Ptr = (T*)other.m_Ptr;
			other.m_Ptr = nullptr;
		}

		~SharedRef()
		{
			RemoveRef();
		}

		SharedRef(const SharedRef<T>& other)
			: m_Ptr(other.m_Ptr)
		{
			AddRef();
		}

		SharedRef& operator=(std::nullptr_t)
		{
			RemoveRef();
			m_Ptr = nullptr;
			return *this;
		}

		SharedRef& operator=(const SharedRef<T>& other)
		{
			other.AddRef();
			RemoveRef();

			m_Ptr = other.m_Ptr;
			return *this;
		}

		template<typename T2>
		SharedRef& operator=(const SharedRef<T2>& other)
		{
			other.AddRef();
			RemoveRef();

			m_Ptr = other.m_Ptr;
			return *this;
		}

		template<typename T2>
		SharedRef& operator=(SharedRef<T2>&& other)
		{
			RemoveRef();

			m_Ptr = other.m_Ptr;
			other.m_Ptr = nullptr;
			return *this;
		}

		operator bool()
		{
			return m_Ptr != nullptr;
		}
		operator bool() const
		{
			return m_Ptr != nullptr;
		}

		T* operator->()
		{
			return m_Ptr;
		}
		const T* operator->() const
		{
			return m_Ptr;
		}

		T& operator*()
		{
			return *m_Ptr;
		}
		const T& operator*() const
		{
			return *m_Ptr;
		}

		T* Ptr()
		{
			return m_Ptr;
		}
		const T* Ptr() const
		{
			return m_Ptr;
		}

		void Reset(T* ptr = nullptr)
		{
			RemoveRef();
			m_Ptr = ptr;
		}

		template<typename T2>
		SharedRef<T2> As()
		{
			return SharedRef<T2>(*this);
		}

		template<typename... Args>
		static SharedRef<T> Create(Args&&... args)
		{
			return SharedRef<T>(new T(std::forward<Args>(args)...));
		}

		template<class T2>
		friend class SharedRef;

	private:
		void AddRef() const
		{
			if (m_Ptr)
			{
				m_Ptr->AddRef();
			}	
		}

		void RemoveRef() const
		{
			if (m_Ptr)
			{
				m_Ptr->RemoveRef();
				if (m_Ptr->GetRefCount() == 0)
				{
					delete m_Ptr;
				}
			}
		}

		T* m_Ptr;
	};
} // namespace Neon
