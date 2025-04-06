#pragma once

namespace Prism
{
template<typename BitType>
class Flags
{
public:
	using MaskType = std::underlying_type_t<BitType>;

	// constructors
	constexpr Flags() = default;

	constexpr Flags(BitType bit) : m_mask(static_cast<MaskType>(bit)) {}

	constexpr Flags(Flags<BitType> const& rhs) = default;

	constexpr explicit Flags(MaskType flags) : m_mask(flags) {}

	// relational operators
	constexpr auto operator<=>(Flags<BitType> const&) const = default;

	// logical operator
	constexpr bool operator!() const
	{
		return !m_mask;
	}

	// bitwise operators
	constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const
	{
		return Flags<BitType>(m_mask & rhs.m_mask);
	}

	constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const
	{
		return Flags<BitType>(m_mask | rhs.m_mask);
	}

	constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const
	{
		return Flags<BitType>(m_mask ^ rhs.m_mask);
	}

	constexpr Flags<BitType> operator~() const
	{
		return Flags<BitType>(~m_mask);
	}

	// assignment operators
	constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) = default;

	constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs)
	{
		m_mask |= rhs.m_mask;
		return *this;
	}

	constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs)
	{
		m_mask &= rhs.m_mask;
		return *this;
	}

	constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs)
	{
		m_mask ^= rhs.m_mask;
		return *this;
	}

	// cast operators
	explicit constexpr operator bool() const
	{
		return !!m_mask;
	}

	explicit constexpr operator MaskType() const
	{
		return m_mask;
	}

	constexpr bool HasAnyFlags(Flags<BitType> flags) const
	{
		return (m_mask & flags.m_mask) != 0;
	}

	constexpr bool HasAllFlags(Flags<BitType> flags) const
	{
		return (m_mask & flags.m_mask) == flags.m_mask;
	}

	constexpr MaskType GetUnderlyingType() const
	{
		return m_mask;
	}

private:
	MaskType m_mask = {};
};
}
