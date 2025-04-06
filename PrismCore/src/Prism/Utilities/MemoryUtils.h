#pragma once

namespace Prism
{
/**
 * Aligns a value to the nearest higher multiple of 'alignment', which must be a power of two.
 *
 * @param  val        The value to align.
 * @param  alignment  The alignment value, must be a power of two.
 *
 * @return The value aligned up to the specified alignment.
 */
template <typename T>
constexpr T Align(T val, int64_t alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "Align expects an integer or pointer type");

	return (T)(((int64_t)val + alignment - 1) & ~(alignment - 1));
}

/**
 * Aligns a value to the nearest lower multiple of 'alignment', which must be a power of two.
 *
 * @param  val        The value to align.
 * @param  alignment  The alignment value, must be a power of two.
 *
 * @return The value aligned down to the specified alignment.
 */
template <typename T>
constexpr T AlignDown(T val, int64_t alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignDown expects an integer or pointer type");

	return (T)(((int64_t)val) & ~(alignment - 1));
}

/**
 * Checks if a pointer is aligned to the specified alignment.
 *
 * @param  val        The value to align.
 * @param  alignment  The alignment value, must be a power of two.
 *
 * @return true if the pointer is aligned to the specified alignment, false otherwise.
 */
template <typename T>
constexpr bool IsAligned(T val, int64_t alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "IsAligned expects an integer or pointer type");

	return !((int64_t)val & (alignment - 1));
}

/**
 * Aligns a value to the nearest higher multiple of 'alignment'.
 *
 * @param  val        The value to align.
 * @param  alignment  The alignment value, can be any arbitrary value.
 *
 * @return The value aligned up to the specified alignment.
 */
template <typename T>
constexpr T AlignArbitrary(T val, int64_t alignment)
{
	static_assert(std::is_integral_v<T> || std::is_pointer_v<T>, "AlignArbitrary expects an integer or pointer type");

	return (T)((((int64_t)val + alignment - 1) / alignment) * alignment);
}
}
