#ifndef HVH_WC_MATH_SIMD_QUAT_H
#define HVH_WC_MATH_SIMD_QUAT_H

#include "vmath_simd.h"
#include "../quat.h"

namespace vmath {


struct simd_quat
{
	union
	{
		__m128 xmm;
		quat base;
	};
};


} // namespace vmath
#endif // HVH_WC_MATH_SIMD_QUAT_H