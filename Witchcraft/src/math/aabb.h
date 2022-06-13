#ifndef HVH_WC_MATH_AABB_H
#define HVH_WC_MATH_AABB_H

#include "vec3.h"

namespace vmath {

// Axis Aligned Bounding Box
class AABB
{
public:

	AABB() = default;
	AABB(const AABB& rhs) = default;

	AABB(vec3 minimum, vec3 maximum)
	: minimum(minimum), maximum(maximum) {}

	vec3 minimum, maximum;

	inline bool isIntersecting(AABB rhs)
	{
		return (this->minimum.x <= rhs.maximum.x &&
				this->maximum.x >= rhs.minimum.x &&
				this->minimum.y <= rhs.maximum.y &&
				this->maximum.y >= rhs.minimum.y &&
				this->minimum.z <= rhs.maximum.z &&
				this->maximum.z >= rhs.minimum.z);
	}

	inline bool isEnclosedBy(AABB rhs)
	{
		return (this->minimum.x >= rhs.minimum.x &&
				this->maximum.x <= rhs.maximum.x &&
				this->minimum.y >= rhs.minimum.y &&
				this->maximum.y <= rhs.maximum.y &&
				this->minimum.z >= rhs.minimum.z &&
				this->maximum.z <= rhs.maximum.z);
	}
};

} // namespace vmath

#endif // HVH_WC_MATH_AABB_H