#pragma once

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
} // namespace Prism::Render::Vulkan
