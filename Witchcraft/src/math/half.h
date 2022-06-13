#ifndef HVH_WC_MATH_HALF_H
#define HVH_WC_MATH_HALF_H

#include "vmath_includes.h"


namespace vmath
{


struct half;

half FloatToHalf(float rhs);
float HalfToFloat(half rhs);

struct half
{
	unsigned short data;

	inline half& operator = (const half&) = default;
		 
	inline half& operator = (float rhs)
		{ return *this = FloatToHalf(rhs); }
	inline half& operator = (int rhs)
		{ return *this = FloatToHalf((float)rhs); }
	inline half& operator = (unsigned int rhs)
		{ return *this = FloatToHalf((float)rhs); }

	inline operator float() const
		{ return HalfToFloat(*this); }
};


} // namespace vmath;
#endif