#ifndef HVH_VMATH_VEC3_H
#define HVH_VMATH_VEC3_H

#include "vmath_includes.h"
#include "vec2.h"

namespace vmath
{

class vec3
{
public:
	//// Constructors
	vec3() = default;
	vec3(const vec3&) = default;
	constexpr vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	constexpr vec3(vec2 xy, float z) : x(xy.data[0]), y(xy.data[1]), z(z) {}
	constexpr vec3(float x, vec2 yz) : x(x), y(yz.data[0]), z(yz.data[1]) {}

	//// Swizzles
private:
	friend class vec4;

	template <typename T, int X, int Y, int Z>
	struct SWIZZLE_READ_ONLY
	{
		inline operator vec3() const
		{
			T* self = (T*)this;
			return{ self->data[X], self->data[Y], self->data[Z] };
		}

		inline vec3 operator + (vec3 rhs)
			{ T* self = (T*)this; return{ self->data[X] + rhs.data[0], self->data[Y] + rhs.data[1], self->data[Z] + rhs.data[2] }; }
		inline vec3 operator - (vec3 rhs)
			{ T* self = (T*)this; return{ self->data[X] - rhs.data[0], self->data[Y] - rhs.data[1], self->data[Z] - rhs.data[2] }; }
		inline vec3 operator * (vec3 rhs)
			{ T* self = (T*)this; return{ self->data[X] * rhs.data[0], self->data[Y] * rhs.data[1], self->data[Z] * rhs.data[2] }; }
		inline vec3 operator / (vec3 rhs)
			{ T* self = (T*)this; return{ self->data[X] / rhs.data[0], self->data[Y] / rhs.data[1], self->data[Z] / rhs.data[2] }; }

		inline vec3 operator + (float rhs)
			{ T* self = (T*)this; return{ self->data[X] + rhs, self->data[Y] + rhs, self->data[Z] + rhs }; }
		inline vec3 operator - (float rhs)
			{ T* self = (T*)this; return{ self->data[X] - rhs, self->data[Y] - rhs, self->data[Z] - rhs }; }
		inline vec3 operator * (float rhs)
			{ T* self = (T*)this; return{ self->data[X] * rhs, self->data[Y] * rhs, self->data[Z] * rhs }; }
		inline vec3 operator / (float rhs)
			{ T* self = (T*)this; return{ self->data[X] / rhs, self->data[Y] / rhs, self->data[Z] / rhs }; }

		inline float magnitude()
			{ return ((vec3)(*this)).magnitude(); }
	};

	template <typename T, int X, int Y, int Z>
	struct SWIZZLE_READWRITE : public SWIZZLE_READ_ONLY<T, X, Y, Z>
	{
		static_assert((X != Y) && (X != Z) && (Y != Z), "Read+Write swizzling cannot be defined for multiple identical elements.");

		inline T& operator = (vec3 rhs)
		{
			T* self = (T*)this;
			self->data[X] = rhs.data[0];
			self->data[Y] = rhs.data[1];
			self->data[Z] = rhs.data[2];
			return *self;
		}
		
		// Any non-const, non-static member functions need to be defined here.

		inline vec3& operator += (vec3 rhs)
			{ T* self = (T*)this; self->data[X] += rhs.data[0]; self->data[Y] += rhs.data[1]; self->data[Z] += rhs.data[2]; return *self; }
		inline vec3& operator -= (vec3 rhs)
			{ T* self = (T*)this; self->data[X] -= rhs.data[0]; self->data[Y] -= rhs.data[1]; self->data[Z] -= rhs.data[2]; return *self; }
		inline vec3& operator *= (vec3 rhs)
			{ T* self = (T*)this; self->data[X] *= rhs.data[0]; self->data[Y] *= rhs.data[1]; self->data[Z] *= rhs.data[2]; return *self; }
		inline vec3& operator /= (vec3 rhs)
			{ T* self = (T*)this; self->data[X] /= rhs.data[0]; self->data[Y] /= rhs.data[1]; self->data[Z] /= rhs.data[2]; return *self; }

		inline vec3& operator += (float rhs)
			{ T* self = (T*)this; self->data[X] += rhs; self->data[Y] += rhs; self->data[Z] += rhs; return *self; }
		inline vec3& operator -= (float rhs)
			{ T* self = (T*)this; self->data[X] -= rhs; self->data[Y] -= rhs; self->data[Z] -= rhs; return *self; }
		inline vec3& operator *= (float rhs)
			{ T* self = (T*)this; self->data[X] *= rhs; self->data[Y] *= rhs; self->data[Z] *= rhs; return *self; }
		inline vec3& operator /= (float rhs)
			{ T* self = (T*)this; self->data[X] /= rhs; self->data[Y] /= rhs; self->data[Z] /= rhs; return *self; }
		
		inline void normalize()
		{
			T* self = (T*)this;
			vec3 temp = { self->data[X], self->data[Y], self->data[Z] };
			temp.normalize();
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2];
		}

		inline void clamp(vec3 minimum, vec3 maximum)
		{
			T* self = (T*)this;
			vec3 temp = { self->data[X], self->data[Y], self->data[Z] };
			temp.clamp(minimum, maximum);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2];
		}

		inline void clamp_magnitude(float max_magnitude)
		{
			T* self = (T*)this;
			vec3 temp = { self->data[X], self->data[Y], self->data[Z] };
			temp.clamp_magnitude(max_magnitude);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2];
		}

		inline void move_towards(vec3 target, float max_dist)
		{
			T* self = (T*)this;
			vec3 temp = { self->data[X], self->data[Y], self->data[Z] };
			temp.move_towards(target, max_dist);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2];
		}
		
	};

public:
	//// Data storage

	union
	{
		struct { float x, y, z; };
		struct { float r, g, b; };
		struct { float s, t, p; };
		float data[3];

		//// vec3 swizzles
		// xyz
		SWIZZLE_READ_ONLY<vec3, 0, 0, 0> xxx;
		SWIZZLE_READ_ONLY<vec3, 0, 0, 1> xxy;
		SWIZZLE_READ_ONLY<vec3, 0, 0, 2> xxz;
		SWIZZLE_READ_ONLY<vec3, 0, 1, 0> xyx;
		SWIZZLE_READ_ONLY<vec3, 0, 1, 1> xyy;
		SWIZZLE_READWRITE<vec3, 0, 1, 2> xyz;
		SWIZZLE_READ_ONLY<vec3, 0, 2, 0> xzx;
		SWIZZLE_READWRITE<vec3, 0, 2, 1> xzy;
		SWIZZLE_READ_ONLY<vec3, 0, 2, 2> xzz;

		SWIZZLE_READ_ONLY<vec3, 1, 0, 0> yxx;
		SWIZZLE_READ_ONLY<vec3, 1, 0, 1> yxy;
		SWIZZLE_READWRITE<vec3, 1, 0, 2> yxz;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 0> yyx;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 1> yyy;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 2> yyz;
		SWIZZLE_READWRITE<vec3, 1, 2, 0> yzx;
		SWIZZLE_READ_ONLY<vec3, 1, 2, 1> yzy;
		SWIZZLE_READ_ONLY<vec3, 1, 2, 2> yzz;

		SWIZZLE_READ_ONLY<vec3, 2, 0, 0> zxx;
		SWIZZLE_READWRITE<vec3, 2, 0, 1> zxy;
		SWIZZLE_READ_ONLY<vec3, 2, 0, 2> zxz;
		SWIZZLE_READWRITE<vec3, 2, 1, 0> zyx;
		SWIZZLE_READ_ONLY<vec3, 2, 1, 1> zyy;
		SWIZZLE_READ_ONLY<vec3, 2, 1, 2> zyz;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 0> zzx;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 1> zzy;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 2> zzz;

		// rgb
		SWIZZLE_READ_ONLY<vec3, 0, 0, 0> rrr;
		SWIZZLE_READ_ONLY<vec3, 0, 0, 1> rrg;
		SWIZZLE_READ_ONLY<vec3, 0, 0, 2> rrb;
		SWIZZLE_READ_ONLY<vec3, 0, 1, 0> rgr;
		SWIZZLE_READ_ONLY<vec3, 0, 1, 1> rgg;
		SWIZZLE_READWRITE<vec3, 0, 1, 2> rgb;
		SWIZZLE_READ_ONLY<vec3, 0, 2, 0> rbr;
		SWIZZLE_READWRITE<vec3, 0, 2, 1> rbg;
		SWIZZLE_READ_ONLY<vec3, 0, 2, 2> rbb;

		SWIZZLE_READ_ONLY<vec3, 1, 0, 0> grr;
		SWIZZLE_READ_ONLY<vec3, 1, 0, 1> grg;
		SWIZZLE_READWRITE<vec3, 1, 0, 2> grb;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 0> ggr;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 1> ggg;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 2> ggb;
		SWIZZLE_READWRITE<vec3, 1, 2, 0> gbr;
		SWIZZLE_READ_ONLY<vec3, 1, 2, 1> gbg;
		SWIZZLE_READ_ONLY<vec3, 1, 2, 2> gbb;

		SWIZZLE_READ_ONLY<vec3, 2, 0, 0> brr;
		SWIZZLE_READWRITE<vec3, 2, 0, 1> brg;
		SWIZZLE_READ_ONLY<vec3, 2, 0, 2> brb;
		SWIZZLE_READWRITE<vec3, 2, 1, 0> bgr;
		SWIZZLE_READ_ONLY<vec3, 2, 1, 1> bgg;
		SWIZZLE_READ_ONLY<vec3, 2, 1, 2> bgb;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 0> bbr;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 1> bbg;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 2> bbb;

		// stp
		SWIZZLE_READ_ONLY<vec3, 0, 0, 0> sss;
		SWIZZLE_READ_ONLY<vec3, 0, 0, 1> sst;
		SWIZZLE_READ_ONLY<vec3, 0, 0, 2> ssp;
		SWIZZLE_READ_ONLY<vec3, 0, 1, 0> sts;
		SWIZZLE_READ_ONLY<vec3, 0, 1, 1> stt;
		SWIZZLE_READWRITE<vec3, 0, 1, 2> stp;
		SWIZZLE_READ_ONLY<vec3, 0, 2, 0> sps;
		SWIZZLE_READWRITE<vec3, 0, 2, 1> spt;
		SWIZZLE_READ_ONLY<vec3, 0, 2, 2> spp;

		SWIZZLE_READ_ONLY<vec3, 1, 0, 0> tss;
		SWIZZLE_READ_ONLY<vec3, 1, 0, 1> tst;
		SWIZZLE_READWRITE<vec3, 1, 0, 2> tsp;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 0> tts;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 1> ttt;
		SWIZZLE_READ_ONLY<vec3, 1, 1, 2> ttp;
		SWIZZLE_READWRITE<vec3, 1, 2, 0> tps;
		SWIZZLE_READ_ONLY<vec3, 1, 2, 1> tpt;
		SWIZZLE_READ_ONLY<vec3, 1, 2, 2> tpp;

		SWIZZLE_READ_ONLY<vec3, 2, 0, 0> pss;
		SWIZZLE_READWRITE<vec3, 2, 0, 1> pst;
		SWIZZLE_READ_ONLY<vec3, 2, 0, 2> psp;
		SWIZZLE_READWRITE<vec3, 2, 1, 0> pts;
		SWIZZLE_READ_ONLY<vec3, 2, 1, 1> ptt;
		SWIZZLE_READ_ONLY<vec3, 2, 1, 2> ptp;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 0> pps;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 1> ppt;
		SWIZZLE_READ_ONLY<vec3, 2, 2, 2> ppp;

		// vec2 swizzles
		// xyz
		vec2::SWIZZLE_READ_ONLY<vec3, 0, 0> xx;
		vec2::SWIZZLE_READWRITE<vec3, 0, 1> xy;
		vec2::SWIZZLE_READWRITE<vec3, 0, 2> xz;
		vec2::SWIZZLE_READWRITE<vec3, 1, 0> yx;
		vec2::SWIZZLE_READ_ONLY<vec3, 1, 1> yy;
		vec2::SWIZZLE_READWRITE<vec3, 1, 2> yz;
		vec2::SWIZZLE_READWRITE<vec3, 2, 0> zx;
		vec2::SWIZZLE_READWRITE<vec3, 2, 1> zy;
		vec2::SWIZZLE_READ_ONLY<vec3, 2, 2> zz;
		// rgb
		vec2::SWIZZLE_READ_ONLY<vec3, 0, 0> rr;
		vec2::SWIZZLE_READWRITE<vec3, 0, 1> rg;
		vec2::SWIZZLE_READWRITE<vec3, 0, 2> rb;
		vec2::SWIZZLE_READWRITE<vec3, 1, 0> gr;
		vec2::SWIZZLE_READ_ONLY<vec3, 1, 1> gg;
		vec2::SWIZZLE_READWRITE<vec3, 1, 2> gb;
		vec2::SWIZZLE_READWRITE<vec3, 2, 0> br;
		vec2::SWIZZLE_READWRITE<vec3, 2, 1> bg;
		vec2::SWIZZLE_READ_ONLY<vec3, 2, 2> bb;
		// stp
		vec2::SWIZZLE_READ_ONLY<vec3, 0, 0> ss;
		vec2::SWIZZLE_READWRITE<vec3, 0, 1> st;
		vec2::SWIZZLE_READWRITE<vec3, 0, 2> sp;
		vec2::SWIZZLE_READWRITE<vec3, 1, 0> ts;
		vec2::SWIZZLE_READ_ONLY<vec3, 1, 1> tt;
		vec2::SWIZZLE_READWRITE<vec3, 1, 2> tp;
		vec2::SWIZZLE_READWRITE<vec3, 2, 0> ps;
		vec2::SWIZZLE_READWRITE<vec3, 2, 1> pt;
		vec2::SWIZZLE_READ_ONLY<vec3, 2, 2> pp;
	};

	//// Operator Overloads

	// Equality test operators.
	inline constexpr bool operator == (vec3 rhs) const
		{ return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z)); }
	inline constexpr bool operator != (vec3 rhs) const
		{ return ((x != rhs.x) || (y != rhs.y) || (z != rhs.z)); }

	// Arithmetic operators (vector with vector)
	inline constexpr vec3 operator + (vec3 rhs) const
		{ return { x+rhs.x, y+rhs.y, z+rhs.z }; }
	inline constexpr vec3 operator - (vec3 rhs) const
		{ return { x-rhs.x, y-rhs.y, z-rhs.z }; }
	inline constexpr vec3 operator * (vec3 rhs) const
		{ return { x*rhs.x, y*rhs.y, z*rhs.z }; }
	inline constexpr vec3 operator / (vec3 rhs) const
		{ return { x/rhs.x, y/rhs.y, z/rhs.z }; }

	// Arithmetic operators (vector with float)
	inline constexpr vec3 operator + (float rhs) const
		{ return { x+rhs, y+rhs, z+rhs }; }
	inline constexpr vec3 operator - (float rhs) const
		{ return { x-rhs, y-rhs, z-rhs }; }
	inline constexpr vec3 operator * (float rhs) const
		{ return { x*rhs, y*rhs, z*rhs }; }
	inline constexpr vec3 operator / (float rhs) const
		{ return { x/rhs, y/rhs, z/rhs }; }

	// Arithmetic operator (float with vector)
	friend inline constexpr vec3 operator + (const float lhs, const vec3 rhs)
		{ return { lhs+rhs.x, lhs+rhs.y, lhs+rhs.z }; }
	friend inline constexpr vec3 operator - (const float lhs, const vec3 rhs)
		{ return { lhs-rhs.x, lhs-rhs.y, lhs-rhs.z }; }
	friend inline constexpr vec3 operator * (const float lhs, const vec3 rhs)
		{ return { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z }; }
	friend inline constexpr vec3 operator / (const float lhs, const vec3 rhs)
		{ return { lhs/rhs.x, lhs/rhs.y, lhs/rhs.z }; }

	// Arithmetic-assignment operators (vector with vector)
	inline vec3& operator += (vec3 rhs)
		{ return *this = *this + rhs; }
	inline vec3& operator -= (vec3 rhs)
		{ return *this = *this - rhs; }
	inline vec3& operator *= (vec3 rhs)
		{ return *this = *this * rhs; }
	inline vec3& operator /= (vec3 rhs)
		{ return *this = *this / rhs; }

	// Arithmetic-assignment operators (vector with float)
	inline vec3& operator += (float rhs)
		{ return *this = *this + rhs; }
	inline vec3& operator -= (float rhs)
		{ return *this = *this - rhs; }
	inline vec3& operator *= (float rhs)
		{ return *this = *this * rhs; }
	inline vec3& operator /= (float rhs)
		{ return *this = *this / rhs; }

	// Negation operator (unary minus)
	inline constexpr vec3 operator - () const
		{ return { -x, -y, -z }; }

	//// Member functions

	inline constexpr bool is_zero() const
		{ return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f)); }

	inline constexpr float sum() const
		{ return x + y + z; }

	inline float magnitude() const
		{ return sqrtf((x*x) + (y*y) + (z*z)); }

	inline float mag() const
		{ return magnitude(); }

	inline float magnitude_squared() const
		{ return (x*x) + (y*y) + (z*z); }

	inline float sqrmag() const
		{ return magnitude_squared(); }

	inline vec3 normalized() const
		{ return *this / magnitude(); }

	inline void normalize()
		{ *this = normalized(); }

	static inline bool close_enough(vec3 lhs, vec3 rhs)
		{ return ((lhs - rhs).magnitude_squared() < 0.001f); }

	static inline vec3 minimum(vec3 lhs, vec3 rhs)
		{ return { fminf(lhs.x, rhs.x), fminf(lhs.y, rhs.y), fminf(lhs.z, rhs.z) }; }

	static inline vec3 maximum(vec3 lhs, vec3 rhs)
		{ return { fmaxf(lhs.x, rhs.x), fmaxf(lhs.y, rhs.y), fmaxf(lhs.z, rhs.z) }; }

	inline vec3 clamped(vec3 lower, vec3 upper) const
		{ return minimum(upper, maximum(lower, *this)); }

	inline void clamp(vec3 lower, vec3 upper)
		{ *this = clamped(lower, upper); }

	inline vec3 clamped_magnitude(float max_magnitude) const
	{
		float mag = magnitude();
		if (mag <= max_magnitude)
			return *this;
		else
			return this->normalized() * max_magnitude;
	}
	inline void clamp_magnitude(float max_magnitude)
		{ *this = clamped_magnitude(max_magnitude); }

	static inline float distance(vec3 a, vec3 b)
		{ return (a-b).magnitude(); }

	static inline float distance_squared(vec3 a, vec3 b)
		{ return (a-b).magnitude_squared(); }

	inline vec3 moved_towards(vec3 target, float max_dist) const
	{
		if (distance(*this, target) <= max_dist)
			return target;
		else
			return *this + ((target - *this).normalized() * max_dist);
	}
	inline void move_towards(vec3 target, float max_dist)
		{ *this = moved_towards(target, max_dist); }

	static inline constexpr float dot(vec3 a, vec3 b)
		{ return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }

	// Cross product: A x B = (a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1)
	static inline constexpr vec3 cross(vec3 a, vec3 b)
		{ return { (a.y*b.z) - (a.z*b.y), (a.z*b.x) - (a.x*b.z), (a.x*b.y) - (a.y*b.x) }; }

	static inline vec3 lerp(vec3 from, vec3 to, float amount)
	{ return (from * (1.0f - amount)) + (to * amount); }

	static inline vec3 ortho_normalize(vec3 a, vec3 b)
	{
		vec3 r = cross(a.normalized(), b.normalized());
		return cross(r, a.normalized());
	}

	static float angle(vec3 a, vec3 b)
		{ return acosf(clampf(dot(a, b) / (a.magnitude() * b.magnitude()), -1.0f, 1.0f)); }

	static float signed_angle(vec3 a, vec3 b)
	{
		float my_angle = angle(a, b);
		if (cross(a, b).z < 0) my_angle = -my_angle;
		return my_angle;
	}

	// Transforms hue/saturation/value to red/green/blue.
	static inline vec3 hsv_to_rgb(float hue, float sat, float val)
	{
		vec3 result;
		result.r = fabsf(hue * 6.0f - 3.0f) - 1.0f;
		result.g = 2 - fabsf(hue * 6.0f - 2.0f);
		result.b = 2 - fabsf(hue * 6.0f - 4.0f);
		result.clamp({ 0,0,0 }, { 1,1,1 });
		return ((result - 1.0f) * sat + 1.0f) * val;
	}

	// I got the implementation for this from Unity.  I honestly don't really know what the hell is going on here.
	static inline vec3 smooth_damp(const vec3 from, const vec3 to, vec3& velocity, float smooth_time, float delta_time, float max_speed)
	{
		if (smooth_time <= 0.0f)
			smooth_time = 0.0001f;
		float num = 2 / smooth_time;
		float num2 = num * delta_time;
		float num3 = 1 / (1 + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2);
		vec3 num4 = from - to;
		vec3 num5 = to;
		float max_dist = max_speed * smooth_time;
		num4.clamp_magnitude(max_dist);
		num5 = from - num4;
		vec3 num7 = ((num4 * num) + velocity) * delta_time;
		velocity = (velocity - num * num7) * num3;
		vec3 num8 = to + (num4 + num7) * num3;
		if (((num5 - from).magnitude() > 0) == (num8.magnitude() > num5.magnitude()))
		{
			num8 = num5;
			velocity = (num8 - num5) / delta_time;
		}

		return num8;
	}

	static inline vec3 spring(vec3 current, vec3& velocity, vec3 target, float delta_time, float tightness)
	{
		vec3 current_to_target = target - current;
		vec3 spring_force = current_to_target * tightness;
		vec3 damping_force = -velocity * 2 * sqrtf(tightness);
		vec3 force = spring_force + damping_force;
		velocity += force * delta_time;
		vec3 displacement = velocity * delta_time;
		return current + displacement;
	}
};

constexpr vec3 VEC3_ZERO = { 0, 0, 0 };
constexpr vec3 VEC3_ONE = { 1, 1, 1 };
constexpr vec3 VEC3_UP = { 0, 0, 1 };
constexpr vec3 VEC3_DOWN = { 0, 0, -1 };
constexpr vec3 VEC3_FORWARD = { 0, 1, 0 };
constexpr vec3 VEC3_BACK = { 0, -1, 0};
constexpr vec3 VEC3_RIGHT = { 1, 0, 0 };
constexpr vec3 VEC3_LEFT = { -1, 0, 0 };

constexpr vec3 VEC3_BLACK = { 0, 0, 0 };
constexpr vec3 VEC3_WHITE = { 1, 1, 1 };
constexpr vec3 VEC3_RED = { 1, 0, 0 };
constexpr vec3 VEC3_GREEN = { 0, 1, 0 };
constexpr vec3 VEC3_BLUE = { 0, 0, 1 };
constexpr vec3 VEC3_YELLOW = { 1, 1, 0 };
constexpr vec3 VEC3_CYAN = { 0, 1, 1 };
constexpr vec3 VEC3_MAGENTA = { 1, 0, 1 };


} // namespace vmath
#endif
