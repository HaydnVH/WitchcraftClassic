#ifndef HVH_WC_MATH_SIMD_MAT4_H
#define HVH_WC_MATH_SIMD_MAT4_H

#include "vmath_simd.h"
#include "../mat4.h"

namespace vmath {


struct simd_mat4
{
	union
	{
		__m128 xmm[4];
		mat4 base;
	};
};

simd_mat4 operator * (const simd_mat4& lhs, const simd_mat4& rhs)
{
	simd_mat4 result;
	__m128 b_line;
	for (int i = 0; i < 4; ++i)
	{
		// Unroll the first iteration of the loop to avoid initializing 'result' to zero.
		b_line = _mm_set1_ps(rhs.base.data2d[i][0]);
		result.xmm[i] = _mm_mul_ps(lhs.xmm[0], b_line);

		for (int j = 1; j < 4; ++j)
		{
			b_line = _mm_set1_ps(rhs.base.data2d[i][j]);
			result.xmm[i] = _mm_add_ps(_mm_mul_ps(lhs.xmm[j], b_line), result.xmm[i]);
		}
	}

	return result;
}


} // namespace vmath
#endif // HVH_WC_MATH_SIMD_MAT4_H