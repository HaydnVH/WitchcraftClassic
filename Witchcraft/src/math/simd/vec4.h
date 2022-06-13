#ifndef HVH_WC_MATH_SIMD_VEC4_H
#define HVH_WC_MATH_SIMD_VEC4_H

#include "vmath_simd.h"
#include "../vec4.h"

namespace vmath {


struct simd_vec4
{
	union
	{
		__m128 xmm;
		vec4 base;
	};
};

inline simd_vec4 operator + (simd_vec4 lhs, simd_vec4 rhs)
	{ return{ _mm_add_ps(lhs.xmm, rhs.xmm) }; }


} // namespace vmath
#endif // HVH_WC_MATH_SIMD_VEC4_H