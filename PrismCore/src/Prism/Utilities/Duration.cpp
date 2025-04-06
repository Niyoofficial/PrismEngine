#include "pcpch.h"
#include "Duration.h"

#include "Prism/Base/Platform.h"

namespace Prism
{
Duration Duration::SecondsToDuration(double seconds)
{
	return Duration((uint64_t)(seconds * (double)Core::Platform::Get().GetPerformanceTicksPerSecond()));
}

Duration Duration::MillisecondsToDuration(double milliseconds)
{
	return Duration(SecondsToDuration(milliseconds / 1000.0));
}

double Duration::GetSeconds() const
{
	return (double)ticks / (double)Core::Platform::Get().GetPerformanceTicksPerSecond();
}

double Duration::GetMilliseconds() const
{
	return GetSeconds() * 1000.0;
}
}
