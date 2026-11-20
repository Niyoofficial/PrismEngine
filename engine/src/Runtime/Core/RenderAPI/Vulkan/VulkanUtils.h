#pragma once

#include <numeric>
#include "Prism/Base/Assert.h"

namespace Prism::Render::Vulkan
{
[[nodiscard]] inline VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment)
{
	PE_ASSERT(alignment != 0, "Alignment must not be zero");

	const VkDeviceSize remainder = value % alignment;

	if (remainder == 0)
	{
		return value;
	}

	const VkDeviceSize padding = alignment - remainder;

	PE_ASSERT(value <= std::numeric_limits<VkDeviceSize>::max() - padding, "Alignment overflow");

	return value + padding;
}

[[nodiscard]] inline VkDeviceSize GetCombinedAlignment(VkDeviceSize aligmentA, VkDeviceSize aligmentB)
{
	PE_ASSERT(aligmentA != 0, "Alignment must not be zero");
	PE_ASSERT(aligmentB != 0, "Alignment must not be zero");

	const VkDeviceSize divisor = std::gcd(aligmentA, aligmentB);

	PE_ASSERT(aligmentA / divisor <= std::numeric_limits<VkDeviceSize>::max() / aligmentB, "Alignment overflow");

	return aligmentA / divisor * aligmentB;
}

[[nodiscard]] inline VkHostAddressRangeEXT MakeHostRange(void* mapped, VkDeviceSize offset, VkDeviceSize size)
{
	return {
		.address = static_cast<uint8_t*>(mapped) + offset,
		.size = static_cast<size_t>(size),
	};
}
} // namespace Prism::Render::Vulkan
