#pragma once
#include <unordered_set>

namespace Prism
{
class RefCounted;

class ReferenceManager
{
public:
	static void AddReference(RefCounted* object)
	{
		s_references.insert(object);

	}

	static void RemoveReference(RefCounted* object)
	{
		s_references.erase(object);
	}

	static bool IsLive(RefCounted* object)
	{
		return s_references.contains(object);
	}

private:
	static std::unordered_set<RefCounted*> s_references;
};

class RefCounted
{
public:
	virtual ~RefCounted() = default;

	void AddRef()
	{
		++m_refCount;

		ReferenceManager::AddReference(this);
	}

	void RemoveRef()
	{
		int32_t newCount = --m_refCount;

		PE_ASSERT(newCount >= 0);

		if (newCount == 0)
			ReferenceManager::RemoveReference(this);
	}

	int32_t GetRefCount() const { return m_refCount.load(); }

private:
	std::atomic<int32_t> m_refCount = 0;
};

template<typename T>
class Ref
{
public:
	Ref() = default;
	Ref(T* object)
		: m_object(object)
	{
		IncRefCount();
	}

	template<typename T2> requires std::is_base_of_v<T, T2>
	Ref(const Ref<T2>& otherRef)
		: Ref<T>(otherRef.m_object)
	{
	}

	template<typename T2> requires std::is_base_of_v<T, T2>
	Ref(Ref<T2>&& otherRef) noexcept
		: m_object(std::move(otherRef.m_object))
	{
		otherRef.m_object = nullptr;
	}

	~Ref()
	{
		DecRefCount();
		m_object = nullptr;
	}

	Ref& operator=(T* object)
	{
		if (object != m_object)
		{
			if (m_object)
				DecRefCount();

			m_object = object;
			IncRefCount();
		}
		return *this;
	}

	template<typename T2> requires std::is_base_of_v<T, T2>
	Ref& operator=(const Ref<T2>& otherRef)
	{
		return *this = static_cast<T*>(otherRef.m_object);
	}

	template<typename T2> requires std::is_base_of_v<T, T2>
	Ref& operator=(Ref<T2>&& otherRef)
	{
		if (m_object != otherRef.m_object)
			Attach(otherRef.Detach());

		return *this;
	}

	bool operator!() const noexcept { return m_object == nullptr; }
	operator bool() const noexcept { return m_object != nullptr; }
	template<typename T2> requires std::is_base_of_v<T, T2>
	bool operator==(const Ref<T2>& otherRef) const noexcept { return m_object == otherRef.m_object; }
	template<typename T2> requires std::is_base_of_v<T, T2>
	bool operator!=(const Ref<T2>& otherRef) const noexcept { return m_object != otherRef.m_object; }

	T& operator*() const noexcept { return *m_object; }
	T* operator->() const noexcept { return Raw(); }

	operator T*() const noexcept { return Raw(); }

	T* Raw() const { return m_object; }

	void Attach(T* object)
	{
		DecRefCount();
		m_object = object;
	}

	T* Detach()
	{
		T* object = m_object;
		m_object = nullptr;
		return object;
	}

private:
	void IncRefCount()
	{
		if (m_object)
			m_object->AddRef();
	}

	void DecRefCount()
	{
		if (m_object)
		{
			m_object->RemoveRef();
			if (m_object->GetRefCount() == 0)
			{
				delete m_object;
				m_object = nullptr;
			}
		}
	}

private:
	T* m_object = nullptr;
};

template<typename T>
class WeakRef
{
public:
	WeakRef() = default;

	WeakRef(Ref<T> ref)
	{
		m_object = ref.Raw();
	}

	WeakRef(T* instance)
	{
		m_object = instance;
	}

	T* operator->() { return m_object; }
	const T* operator->() const { return m_object; }

	T& operator*() { return *m_object; }
	const T& operator*() const { return *m_object; }

	bool IsValid() const { return m_object ? ReferenceManager::IsLive(m_object) : false; }
	operator bool() const { return IsValid(); }

private:
	T* m_object = nullptr;
};
}
