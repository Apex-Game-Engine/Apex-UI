class UiBase;

template <typename T>
class UiPtr
{
public:
	UiPtr(std::nullptr_t) : m_ptr(nullptr) {}

    UiPtr(T* ptr) : m_ptr(ptr)
    {
		LOG_FUNC();
		if (m_ptr)
        	m_ptr->OnCreateBase();
    }

	~UiPtr()
    {
		if (m_ptr)
        	m_ptr->OnDestroyBase();
    }

	UiPtr(UiPtr const&) = delete;

	UiPtr(UiPtr&& other) : m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	template <typename U> requires (std::is_same<T, UiBase>::value)
	UiPtr(UiPtr<U>&& other) : m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	UiPtr& operator=(UiPtr const&) = delete;

	UiPtr& operator=(UiPtr&& other)
	{
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
	}

	template <typename U> requires (std::is_same<T, UiBase>::value)
	UiPtr& operator=(UiPtr<U>&& other)
	{
		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
	}

    T* operator->()
    {
        return m_ptr;
    }

    const T* operator->() const
    {
        return m_ptr;
    }

    const T& operator*() const
    {
        return *m_ptr;
    }

    UiBase* GetPtr()
    {
        return m_ptr;
    }

    const UiBase* GetPtr() const
    {
        return m_ptr;
    }

private:
    T* m_ptr;
};

using UiBasePtr = UiPtr<UiBase>;