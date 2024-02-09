#pragma once

namespace Prism
{
class Duration
{
public:
	explicit Duration(uint64_t ticks);

	static Duration SecondsToDuration(double seconds);
	static Duration MillisecondsToDuration(double milliseconds);

	double GetSeconds();
	double GetMilliseconds();

private:
	uint64_t m_ticks = 0;
};
}
