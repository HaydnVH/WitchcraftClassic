#ifndef HVH_VMATH_VEC2_H
#define HVH_VMATH_VEC2_H

#include "vmath_includes.h"

namespace vmath
{

class vec2
{
public:
	//// Constructors
	vec2() = default;
	vec2(const vec2&) = default;
	constexpr vec2(float x, float y) : x(x), y(y) {}

private:
	friend class vec3;
	friend class vec4;

	// Here we define swizzle proxies.
	template <typename T, int X, int Y>
	struct SWIZZLE_READ_ONLY
	{
		inline operator vec2() const
		{
			T* self = (T*)this;
			return{ self->data[X], self->data[Y] };
		}

		inline vec2 operator + (vec2 rhs)
			{ T* self = (T*)this; return{ self->data[X] + rhs.data[0], self->data[Y] + rhs.data[1] }; }
		inline vec2 operator - (vec2 rhs)
			{ T* self = (T*)this; return{ self->data[X] - rhs.data[0], self->data[Y] - rhs.data[1] }; }
		inline vec2 operator * (vec2 rhs)
			{ T* self = (T*)this; return{ self->data[X] * rhs.data[0], self->data[Y] * rhs.data[1] }; }
		inline vec2 operator / (vec2 rhs)
			{ T* self = (T*)this; return{ self->data[X] / rhs.data[0], self->data[Y] / rhs.data[1] }; }

		inline vec2 operator + (float rhs)
			{ T* self = (T*)this; return{ self->data[X] + rhs, self->data[Y] + rhs }; }
		inline vec2 operator - (float rhs)
			{ T* self = (T*)this; return{ self->data[X] - rhs, self->data[Y] - rhs }; }
		inline vec2 operator * (float rhs)
			{ T* self = (T*)this; return{ self->data[X] * rhs, self->data[Y] * rhs }; }
		inline vec2 operator / (float rhs)
			{ T* self = (T*)this; return{ self->data[X] / rhs, self->data[Y] / rhs }; }
	};

	template <typename T, int X, int Y>
	struct SWIZZLE_READWRITE : public SWIZZLE_READ_ONLY<T, X, Y>
	{
		static_assert(X != Y, "Read+Write swizzling cannot be defined for multiple identical elements.");

		inline T& operator = (vec2 rhs)
		{
			T* self = (T*)this;
			self->data[X] = rhs.data[0];
			self->data[Y] = rhs.data[1];
			return *self;
		}
		
		// Any non-const, non-static member functions need to be defined here.

		inline vec2& operator += (vec2 rhs)
			{ T* self = (T*)this; self->data[X] += rhs.data[0]; self->data[Y] += rhs.data[1]; return (vec2&)*self; }
		inline vec2& operator -= (vec2 rhs)
			{ T* self = (T*)this; self->data[X] -= rhs.data[0]; self->data[Y] -= rhs.data[1]; return (vec2&)*self; }
		inline vec2& operator *= (vec2 rhs)
			{ T* self = (T*)this; self->data[X] *= rhs.data[0]; self->data[Y] *= rhs.data[1]; return (vec2&)*self; }
		inline vec2& operator /= (vec2 rhs)
			{ T* self = (T*)this; self->data[X] /= rhs.data[0]; self->data[Y] /= rhs.data[1]; return (vec2&)*self; }

		inline vec2& operator += (float rhs)
			{ T* self = (T*)this; self->data[X] += rhs; self->data[Y] += rhs; return (vec2&)*self; }
		inline vec2& operator -= (float rhs)
			{ T* self = (T*)this; self->data[X] -= rhs; self->data[Y] -= rhs; return (vec2&)*self; }
		inline vec2& operator *= (float rhs)
			{ T* self = (T*)this; self->data[X] *= rhs; self->data[Y] *= rhs; return (vec2&)*self; }
		inline vec2& operator /= (float rhs)
			{ T* self = (T*)this; self->data[X] /= rhs; self->data[Y] /= rhs; return (vec2&)*self; }

		inline void normalize()
		{
			T* self = (T*)this;
			vec2 temp = { self->data[X], self->data[Y] };
			temp.normalize();
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1];
		}

		inline void clamp(vec2 minimum, vec2 maximum)
		{
			T* self = (T*)this;
			vec2 temp = { self->data[X], self->data[Y] };
			temp.clamp(minimum, maximum);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1];
		}

		inline void clamp_magnitude(float max_magnitude)
		{
			T* self = (T*)this;
			vec2 temp = { self->data[X], self->data[Y] };
			temp.clamp_magnitude(max_magnitude);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1];
		}

		inline void move_towards(vec2 target, float max_dist)
		{
			T* self = (T*)this;
			vec2 temp = { self->data[X], self->data[Y] };
			temp.move_towards(target, max_dist);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1];
		}
	};

public:

	//// Data storage
	union
	{
		struct { float x, y; };
		struct { float r, g; };
		struct { float s, t; };
		float data[2];

		//// Swizzles
		// xy
		SWIZZLE_READ_ONLY<vec2, 0, 0> xx;
		SWIZZLE_READWRITE<vec2, 0, 1> xy;
		SWIZZLE_READ_ONLY<vec2, 1, 1> yy;
		SWIZZLE_READWRITE<vec2, 1, 0> yx;
		// rg
		SWIZZLE_READ_ONLY<vec2, 0, 0> rr;
		SWIZZLE_READWRITE<vec2, 0, 1> rg;
		SWIZZLE_READ_ONLY<vec2, 1, 1> gg;
		SWIZZLE_READWRITE<vec2, 1, 0> gr;
		// st
		SWIZZLE_READ_ONLY<vec2, 0, 0> ss;
		SWIZZLE_READWRITE<vec2, 0, 1> st;
		SWIZZLE_READ_ONLY<vec2, 1, 1> tt;
		SWIZZLE_READWRITE<vec2, 1, 0> ts;
	};

	//// Operator Overloads

	// Equality test operators.
	inline constexpr bool operator == (vec2 rhs) const
		{ return ((x == rhs.x) && (y == rhs.y)); }
	inline constexpr bool operator != (vec2 rhs) const
		{ return ((x != rhs.x) || (y != rhs.y)); }

	// Arithmetic operators (vector with vector)
	inline constexpr vec2 operator + (vec2 rhs) const
		{ return { x+rhs.x, y+rhs.y }; }
	inline constexpr vec2 operator - (vec2 rhs) const
		{ return { x-rhs.x, y-rhs.y }; }
	inline constexpr vec2 operator * (vec2 rhs) const
		{ return { x*rhs.x, y*rhs.y }; }
	inline constexpr vec2 operator / (vec2 rhs) const
		{ return { x / rhs.x, y / rhs.y }; }

	// Arithmetic operators (vector with float)
	inline constexpr vec2 operator + (float rhs) const
		{ return { x+rhs, y+rhs }; }
	inline constexpr vec2 operator - (float rhs) const
		{ return { x-rhs, y-rhs }; }
	inline constexpr vec2 operator * (float rhs) const
		{ return { x*rhs, y*rhs }; }
	inline constexpr vec2 operator / (float rhs) const
		{ return { x/rhs, y/rhs }; }

	// Arithmetic operator (float with vector)
	friend inline constexpr vec2 operator + (const float lhs, const vec2 rhs)
		{ return { lhs+rhs.x, lhs+rhs.y }; }
	friend inline constexpr vec2 operator - (const float lhs, const vec2 rhs)
		{ return { lhs-rhs.x, lhs-rhs.y }; }
	friend inline constexpr vec2 operator * (const float lhs, const vec2 rhs)
		{ return { lhs*rhs.x, lhs*rhs.y }; }
	friend inline constexpr vec2 operator / (const float lhs, const vec2 rhs)
		{ return { lhs/rhs.x, lhs/rhs.y }; }

	// Arithmetic-assignment operators (vector with vector)
	inline vec2& operator += (vec2 rhs)
		{ return *this = *this + rhs; }
	inline vec2& operator -= (vec2 rhs)
		{ return *this = *this - rhs; }
	inline vec2& operator *= (vec2 rhs)
		{ return *this = *this * rhs; }
	inline vec2& operator /= (vec2 rhs)
		{ return *this = *this / rhs; }

	// Arithmetic-assignment operators (vector with float)
	inline vec2& operator += (float rhs)
		{ return *this = *this + rhs; }
	inline vec2& operator -= (float rhs)
		{ return *this = *this - rhs; }
	inline vec2& operator *= (float rhs)
		{ return *this = *this * rhs; }
	inline vec2& operator /= (float rhs)
		{ return *this = *this / rhs; }

	// Negation operator (unary minus)
	inline constexpr vec2 operator - () const
		{ return { -x, -y }; }

	//// Member functions

	inline constexpr bool is_zero() const
		{ return ((x == 0.0f) && (y == 0.0f)); }

	inline constexpr float sum() const
		{ return x + y; }

	inline float magnitude() const
		{ return sqrtf((x*x) + (y*y)); }

	inline float mag() const
		{ return magnitude(); }

	inline float magnitude_squared() const
		{ return (x*x) + (y*y); }

	inline float sqrmag() const
		{ return magnitude_squared(); }

	inline vec2 normalized() const
		{ return *this / magnitude(); }

	inline void normalize()
		{ *this = normalized(); }

	static inline bool close_enough(vec2 lhs, vec2 rhs)
		{ return ((lhs - rhs).magnitude_squared() < 0.001f); }

	static inline vec2 minimum(vec2 lhs, vec2 rhs)
		{ return { fminf(lhs.x, rhs.x), fminf(lhs.y, rhs.y) }; }

	static inline vec2 maximum(vec2 lhs, vec2 rhs)
		{ return { fmaxf(lhs.x, rhs.x), fmaxf(lhs.y, rhs.y) }; }

	inline vec2 clamped(vec2 lower, vec2 upper) const
		{ return minimum(upper, maximum(lower, *this)); }

	inline void clamp(vec2 lower, vec2 upper)
		{ *this = clamped(lower, upper); }

	inline vec2 clamped_magnitude(float max_magnitude) const
	{
		float mag = magnitude();
		if (mag <= max_magnitude)
			return *this;
		else
			return this->normalized() * max_magnitude;
	}
	inline void clamp_magnitude(float max_magnitude)
		{ *this = clamped_magnitude(max_magnitude); }

	static inline float distance(vec2 a, vec2 b)
		{ return (a-b).magnitude(); }

	static inline float distance_squared(vec2 a, vec2 b)
		{ return (a-b).magnitude_squared(); }

	inline vec2 moved_towards(vec2 target, float max_dist) const
	{
		if (distance(*this, target) <= max_dist)
			return target;
		else
			return *this + ((target - *this).normalized() * max_dist);
	}
	inline void move_towards(vec2 target, float max_dist)
		{ *this = moved_towards(target, max_dist); }

	static inline constexpr float determinant(vec2 a, vec2 b)
		{ return (a.x * b.y) - (a.y * b.x); }

	static inline constexpr float det(vec2 a, vec2 b)
		{ return determinant(a, b); }

	static inline constexpr float dot(vec2 a, vec2 b)
		{ return (a.x * b.x) + (a.y * b.y); }

	static inline vec2 lerp(vec2 from, vec2 to, float amount)
		{ return (from * (1.0f - amount)) + (to * amount); }

	static inline float angle(vec2 from, vec2 to)
		{ return atan2f(det(from, to), dot(from, to)); }

	static vec2 smooth_damp(vec2 from, vec2 to, vec2& velocity, float smooth_time, float delta_time, float max_speed)
	{
		if (smooth_time <= 0.0f)
			smooth_time = 0.0001f;
		float num = 2 / smooth_time;
		float num2 = num * delta_time;
		float num3 = 1 / (1 + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2);
		vec2 num4 = from - to;
		vec2 num5 = to;
		float max_dist = max_speed * smooth_time;
		num4.clamp_magnitude(max_dist);
		num5 = from - num4;
		vec2 num7 = ((num4 * num) + velocity) * delta_time;
		velocity = (velocity - num * num7) * num3;
		vec2 num8 = to + (num4 + num7) * num3;
		if (((num5 - from).magnitude() > 0) == (num8.magnitude() > num5.magnitude()))
		{
			num8 = num5;
			velocity = (num8 - num5) / delta_time;
		}

		return num8;
	}
};

constexpr vec2 VEC2_ZERO = { 0, 0 };
constexpr vec2 VEC2_ONE = { 1, 1 };

} // namespace vmath
#endif
