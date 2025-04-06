#pragma once

#include "Prism/Base/Base.h"

namespace Prism
{

/* Singleton that will be created on its first access and can be destroyed on demand */
template<class T>
class LazySingleton final
{
public:
	static T& Get()
	{
		return GetSingleton().GetValue();
	}

	static T* TryGet()
	{
		return GetSingleton().TryGetValue();
	}

	static void Destroy()
	{
		return GetSingleton().Reset();
	}

private:
	LazySingleton()
		: m_ptr(new(m_data) T)
	{
	}

	~LazySingleton()
	{
		Reset();
	}

	static LazySingleton& GetSingleton()
	{
		static LazySingleton s_singleton;
		return s_singleton;
	}

	T* TryGetValue()
	{
		return m_ptr;
	}

	T& GetValue()
	{
		PE_ASSERT(m_ptr);
		return *m_ptr;
	}

	void Reset()
	{
		if (m_ptr)
		{
			m_ptr->~T();
			m_ptr = nullptr;
		}
	}

private:
	alignas(T)
	uint8_t m_data[sizeof(T)] = {};
	T* m_ptr = nullptr;
};
}
