#ifndef HVH_WC_MATH_VMATHINCLUDES_H
#define HVH_WC_MATH_VMATHINCLUDES_H

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <float.h>

constexpr const float PI = 3.14159265359f;
constexpr const float QUARTER_PI = PI / 4;
constexpr const float HALF_PI = PI / 2;
constexpr const float PI2 = PI * 2;
constexpr const float PI4 = PI * 4;

constexpr float const TAU = 6.28318530718f;
constexpr float const QUARTER_TAU = TAU / 4;
constexpr float const HALF_TAU = TAU / 2;
constexpr float const TAU2 = TAU * 2;
constexpr float const TAU4 = TAU * 4;

constexpr const float TO_RADIANS = (TAU / 360.0f);
constexpr const float TO_DEGREES = (360.0f / TAU);

inline float springf(float current, float& velocity, float target, float delta_time, float tightness)
{
	float current_to_target = target - current;
	float spring_force = current_to_target * tightness;
	float damping_force = -velocity * 2 * sqrtf(tightness);
	float force = spring_force + damping_force;
	velocity += force * delta_time;
	float displacement = velocity * delta_time;
	return current + displacement;
}

inline float lerp(float from, float to, float amount)
{
	return (from * (1.0f - amount)) + (to * amount);
}

inline float clampf(float value, float minimum, float maximum)
{
	return fminf(maximum, fmaxf(minimum, value));
}

inline bool nearly_equals(float a, float b, float epsilon = 0.000001f)
{
	// http://floating-point-gui.de/errors/comparison/
	float absA = fabsf(a);
	float absB = fabsf(b);
	float diff = fabsf(a - b);

	if (a == b) // shortcut, handles infinities
		return true;
	else if (a == 0 || b == 0 || diff < FLT_EPSILON)
		return diff < epsilon;
	else
		return (diff / fminf(absA + absB, FLT_MAX)) < epsilon;
}

inline int8_t float_to_byte(float in)
{
	if (in >= 1.0f)
		return SCHAR_MAX;
	else if (in <= -1.0f)
		return SCHAR_MIN;
	else if (in >= 0)
		return (int8_t)(in * SCHAR_MAX);
	else // if (in < 0)
		return (int8_t)(in * -SCHAR_MIN);
}

inline uint8_t float_to_ubyte(float in)
{
	if (in >= 1.0f)
		return UCHAR_MAX;
	else if (in <= 0.0f)
		return 0;
	else
		return (uint8_t)(in * UCHAR_MAX);
}

inline float byte_to_float(int8_t in)
{
	if (in >= 0)
		return (float)in / (float)SCHAR_MAX;
	else // if (in < 0)
		return (float)in / (float)-SCHAR_MIN;
}

inline float ubyte_to_float(uint8_t in)
{
	return (float)in / (float)UCHAR_MAX;
}

// Returns +1 if the input is positive (or zero), or -1 if it is negative.
inline float signof(float in)
{
	if (in >= 0)
		return 1.0f;
	else
		return -1.0f;
}

// Returns +1 if the input is positive, 0 if the input is zero, or -1 if the input is negative.
inline float signof_or_zero(float in)
{
	if (in > 0.0f)
		return 1.0f;
	else if (in < 0.0f)
		return -1.0f;
	else // if (in == 0.0f)
		return 0.0f;
}

#endif 