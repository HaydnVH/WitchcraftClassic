#ifndef HVH_WC_MATH_MAT3_H
#define HVH_WC_MATH_MAT3_H

#include "vec3.h"


namespace vmath
{


struct mat3
{
	union
	{
		float data1d[9];
		float data2d[3][3];
	};
};

constexpr const mat3 MAT3_ZERO = {0};
constexpr const mat3 MAT3_IDENTITY = { 1,0,0,
									   0,1,0,
									   0,0,1 };


} // namespace vmath
#endif
