#pragma once

namespace Prism
{
struct Duration
{
public:
	constexpr Duration() = default;
	constexpr explicit Duration(uint64_t inTicks)
		: ticks(inTicks)
	{
		
	}

	static Duration SecondsToDuration(double seconds);
	static Duration MillisecondsToDuration(double milliseconds);

	double GetSeconds() const;
	double GetMilliseconds() const;

	constexpr Duration operator+(Duration other) const
	{
		return Duration(ticks + other.ticks);
	}

	constexpr Duration operator-(Duration other) const
	{
		return Duration(ticks - other.ticks);
	}

	constexpr Duration operator*(Duration other) const
	{
		return Duration(ticks * other.ticks);
	}

	constexpr Duration operator/(Duration other) const
	{
		return Duration(ticks / other.ticks);
	}

	constexpr auto operator<=>(const Duration& other) const = default;

public:
	uint64_t ticks = 0;
};
}
