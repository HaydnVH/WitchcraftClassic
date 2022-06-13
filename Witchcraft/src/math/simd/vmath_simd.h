#ifndef HVH_WC_MATH_SIMD_VMATHSIMD_H
#define HVH_WC_MATH_SIMD_VMATHSIMD_H

#include <xmmintrin.h>

inline __m128 operator + (__m128 lhs, __m128 rhs)
	{ return _mm_add_ps(lhs, rhs); }
inline __m128 operator - (__m128 lhs, __m128 rhs)
	{ return _mm_sub_ps(lhs, rhs); }
inline __m128 operator * (__m128 lhs, __m128 rhs)
	{ return _mm_mul_ps(lhs, rhs); }
inline __m128 operator / (__m128 lhs, __m128 rhs)
	{ return _mm_div_ps(lhs, rhs); }

inline __m128& operator += (__m128& lhs, __m128 rhs)
	{ return lhs = _mm_add_ps(lhs, rhs); }
inline __m128& operator -= (__m128& lhs, __m128 rhs)
	{ return lhs = _mm_sub_ps(lhs, rhs); }
inline __m128& operator *= (__m128& lhs, __m128 rhs)
	{ return lhs = _mm_mul_ps(lhs, rhs); }
inline __m128& operator /= (__m128& lhs, __m128 rhs)
{
	return lhs = _mm_div_ps(lhs, rhs);
}

#endif // HVH_WC_MATH_SIMD_VMATHSIMD_H