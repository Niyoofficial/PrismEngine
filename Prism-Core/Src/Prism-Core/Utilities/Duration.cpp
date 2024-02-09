#include "pcpch.h"
#include "Duration.h"

#include "Prism-Core/Base/Platform.h"

namespace Prism
{
Duration::Duration(uint64_t ticks)
	: m_ticks(ticks)
{
}

Duration Duration::SecondsToDuration(double seconds)
{
	return Duration((uint64_t)(seconds * (double)Core::Platform::Get().GetPerformanceTicksPerSecond()));
}

Duration Duration::MillisecondsToDuration(double milliseconds)
{
	return Duration(SecondsToDuration(milliseconds / 1000.0));
}

double Duration::GetSeconds()
{
	return (double)m_ticks / (double)Core::Platform::Get().GetPerformanceTicksPerSecond();
}

double Duration::GetMilliseconds()
{
	return GetSeconds() * 1000.0;
}
}
