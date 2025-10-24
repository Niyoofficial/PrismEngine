#pragma once

namespace Prism::Utils::Private
{
/**
 * Not meant for direct consumption : use ON_SCOPE_EXIT instead.
 *
 * RAII class that calls a lambda when it is destroyed.
 */
template <typename T>
class ScopeGuard
{
public:
	ScopeGuard(ScopeGuard&&) = delete;
	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator=(ScopeGuard&&) = delete;
	ScopeGuard& operator=(const ScopeGuard&) = delete;

	// Given a lambda, constructs an RAII scope guard.
	explicit ScopeGuard(T&& InFunc)
		: func((T&&)InFunc)
	{
	}

	// Causes the lambda to be executed.
	~ScopeGuard()
	{
		func();
	}

private:
	// The lambda to be executed when this guard goes out of scope.
	T func;
};

struct ScopeGuardSyntaxSupport
{
	template <typename T>
	ScopeGuard<T> operator+(T&& func)
	{
		return ScopeGuard<T>((T&&)func);
	}
};
}

/**
 * Enables a lambda to be executed on scope exit.
 *
 * Example:
 *    {
 *      FileHandle* handle = GetFileHandle();
 *      ON_SCOPE_EXIT
 *      {
 *          CloseFile(handle);
 *      };
 *
 *      DoSomethingWithFile(handle);
 *
 *      // File will be closed automatically no matter how the scope is exited, e.g.:
 *      // * Any return statement.
 *      // * break or continue (if the scope is a loop body).
 *      // * An exception is thrown outside the block.
 *      // * Execution reaches the end of the block.
 *    }
 */
#define ON_SCOPE_EXIT const auto PREPROCESSOR_JOIN(ScopeGuard_, __LINE__) = ::Prism::Utils::Private::ScopeGuardSyntaxSupport() + [&]()
