#ifndef HVH_WC_MATH_SIMD_4VEC3_H
#define HVH_WC_MATH_SIMD_4VEC3_H

#include "vmath_simd.h"

#include "../vec3.h"

namespace vmath {


/*

Instead of storing data in a pure struct-of-arrays, such as:
[xxxxxxxxxxx...]
[yyyyyyyyyyy...]
[zzzzzzzzzzz...]
Or as a pure array-of-structs, such as:
[xyzxyzxyzxyzxyzxyzyxz...]
We can store it in a specialized "array-of-strucs-of-arrays", such as:
[xxxxyyyyzzzzxxxxyyyyzzzzxxxxyyyyzzzz...]
This gives us all of the advantages of using a struct-of-arrays (such as easy SIMD computation),
while still giving us the advantages of an array-of-structs (including only loading one cache line to read a vector).

The problem with SOA is that vectors are typically used as a whole, and not one dimension at a time,
so loading in a complete vector would require loading 3 cache lines (both worst and best case).

The problem with AOS is that vectors can never be properly aligned for SIMD use,
so implementing SIMD optimizations would require us to shuffle data around constantly,
potentially negating any performance boost that SIMD would give us in the first place.

Using this structure, we can arrange data in a way that SIMD can use immediately with no shuffling,
while still only having to load 1 or 2 cache lines (depending on alignment) when we want to access a single vector.
It also allows us to keep the implementation as simple as possible,
essentially using the exact same logic as normal vectors and matrices.

*/

struct simd_4vec3
{
	union
	{
		struct { __m128 x, y, z; };
		__m128 data[3];
		float fdata[3][4];
	};
};

inline vec3 get_vec3(const simd_4vec3& in, uint32_t i)
{
	return{ in.fdata[0][i], in.fdata[1][i], in.fdata[2][i] };
}

inline simd_4vec3 operator + (const simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return{ _mm_add_ps(lhs.x, rhs.x), _mm_add_ps(lhs.y, rhs.y), _mm_add_ps(lhs.z, rhs.z) }; }
inline simd_4vec3 operator - (const simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return{ _mm_sub_ps(lhs.x, rhs.x), _mm_sub_ps(lhs.y, rhs.y), _mm_sub_ps(lhs.z, rhs.z) }; }
inline simd_4vec3 operator * (const simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return{ _mm_mul_ps(lhs.x, rhs.x), _mm_mul_ps(lhs.y, rhs.y), _mm_mul_ps(lhs.z, rhs.z) }; }
inline simd_4vec3 operator / (const simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return{ _mm_div_ps(lhs.x, rhs.x), _mm_div_ps(lhs.y, rhs.y), _mm_div_ps(lhs.z, rhs.z) }; }

inline simd_4vec3 operator + (const simd_4vec3& lhs, __m128 rhs)
	{ return{ _mm_add_ps(lhs.x, rhs), _mm_add_ps(lhs.y, rhs), _mm_add_ps(lhs.z, rhs) }; }
inline simd_4vec3 operator - (const simd_4vec3& lhs, __m128 rhs)
	{ return{ _mm_sub_ps(lhs.x, rhs), _mm_sub_ps(lhs.y, rhs), _mm_sub_ps(lhs.z, rhs) }; }
inline simd_4vec3 operator * (const simd_4vec3& lhs, __m128 rhs)
	{ return{ _mm_mul_ps(lhs.x, rhs), _mm_mul_ps(lhs.y, rhs), _mm_mul_ps(lhs.z, rhs) }; }
inline simd_4vec3 operator / (const simd_4vec3& lhs, __m128 rhs)
	{ return{ _mm_div_ps(lhs.x, rhs), _mm_div_ps(lhs.y, rhs), _mm_div_ps(lhs.z, rhs) }; }

inline simd_4vec3& operator += (simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return lhs = { _mm_add_ps(lhs.x, rhs.x), _mm_add_ps(lhs.y, rhs.y), _mm_add_ps(lhs.z, rhs.z) }; }
inline simd_4vec3& operator -= (simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return lhs = { _mm_sub_ps(lhs.x, rhs.x), _mm_sub_ps(lhs.y, rhs.y), _mm_sub_ps(lhs.z, rhs.z) }; }
inline simd_4vec3& operator *= (simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return lhs = { _mm_mul_ps(lhs.x, rhs.x), _mm_mul_ps(lhs.y, rhs.y), _mm_mul_ps(lhs.z, rhs.z) }; }
inline simd_4vec3& operator /= (simd_4vec3& lhs, const simd_4vec3& rhs)
	{ return lhs = { _mm_div_ps(lhs.x, rhs.x), _mm_div_ps(lhs.y, rhs.y), _mm_div_ps(lhs.z, rhs.z) }; }

inline __m128 magnitude(const simd_4vec3& in)
	{ return _mm_sqrt_ps((in.x*in.x) + (in.y*in.y) + (in.z*in.z)); }

inline void normalize(simd_4vec3& in)
	{ in = in / magnitude(in); }
inline simd_4vec3 normalized(const simd_4vec3& in)
	{ return in / magnitude(in); }



} // namespace vmath
#endif // HVH_WC_MATH_SIDM_4VEC3_H