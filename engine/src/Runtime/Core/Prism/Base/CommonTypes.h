#include "glm/gtx/compatibility.hpp"

namespace Prism
{
template<typename T>
struct Box
{
	T location = {};
	T size = {};
};

using Box3I = Box<glm::int3>;

template<typename T>
struct Bounds
{
	void operator+=(const Bounds<T>& other)
	{
		min = glm::min(min, other.min);
		max = glm::max(max, other.max);
	}

	void operator+=(const T& other)
	{
		min = glm::min(min, other);
		max = glm::max(max, other);
	}

	float GetRadius() const
	{
		return glm::max(glm::length(min), glm::length(max));
	}

	T min = std::numeric_limits<T>::max();
	T max = std::numeric_limits<T>::min();
};

using Bounds3i = Bounds<glm::int3>;
using Bounds3f = Bounds<glm::float3>;
}
