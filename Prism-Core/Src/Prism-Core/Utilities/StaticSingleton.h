#pragma once

namespace Prism
{
/* Singleton that will be created as a static variable */
template<typename T>
class StaticSingleton
{
public:
	static T& Get()
	{
		return s_singleton;
	}

private:
	static inline T s_singleton;
};

/* Singleton that is manually created and destroyed */
template<typename T>
class StaticPointerSingleton
{
public:
	static void Create()
	{
		s_singleton = (T*)malloc(sizeof(T));
		new (&s_singleton) T;
	}

	/* Don't hold any reference to the singleton yourself after passing it to this function! */
	static void Create(T* singleton)
	{
		s_singleton = singleton;
	}

	static void Destroy()
	{
		PE_ASSERT(s_singleton != nullptr, "Singleton was already destroyed or never created!");

		delete s_singleton;
		s_singleton = nullptr;
	}

	static void TryDestroy()
	{
		if (s_singleton != nullptr)
		{
			delete s_singleton;
			s_singleton = nullptr;
		}
	}

	/* Do not call after Destroy!
	 * Do not store any reference to the singleton after getting it from this function,
	 * it may be destroyed at any time and your existing references will become garbage pointers */
	static T& Get()
	{
		PE_ASSERT(s_singleton != nullptr, "Singleton is null!");

		return *s_singleton;
	}

	/* Do not store any reference to the singleton after getting it from this function,
	 * it may be destroyed at any time and your existing references will become garbage pointers */
	static T* TryGet()
	{
		return s_singleton;
	}

private:
	static inline T* s_singleton = nullptr;
};
}