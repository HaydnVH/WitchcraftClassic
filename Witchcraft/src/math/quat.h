#ifndef HVH_WC_MATH_QUAT_H
#define HVH_WC_MATH_QUAT_H

#include "vmath_includes.h"
#include "vec3.h"
#include "vec4.h"

namespace vmath
{

class quat
{
public:
	//// Constructors
	quat() = default;
	quat(const quat& rhs) = default;
	constexpr quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	//// Data Storage
	union
	{
		struct { float x, y, z, w; };
		float data[4];
	};

	//// Operator Overloads

	// Quality test operators
	inline constexpr bool operator == ( quat rhs) const
		{ return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w)); }
	inline constexpr bool operator != (quat rhs) const
		{ return ((x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w)); }

	// Arithmetic Operators (quat with quat)
	inline constexpr quat operator + (quat rhs) const
		{ return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
	inline constexpr quat operator - (quat rhs) const
		{ return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
	inline constexpr quat operator * (quat rhs) const
	{
		return{ ((rhs.w * x) + (rhs.x * w) + (rhs.y * z) - (rhs.z * y)),
				((rhs.w * y) + (rhs.y * w) + (rhs.z * x) - (rhs.x * z)),
				((rhs.w * z) + (rhs.z * w) + (rhs.x * y) - (rhs.y * x)),
				((rhs.w * w) - (rhs.x * x) - (rhs.y * y) - (rhs.z * z)) };
	}

	// Arithmetic operators (quat with float)
	inline constexpr quat operator + (float rhs) const
		{ return { x+rhs, y+rhs, z+rhs, w+rhs }; }
	inline constexpr quat operator - (float rhs) const
		{ return { x-rhs, y-rhs, z-rhs, w-rhs }; }
	inline constexpr quat operator * (float rhs) const
		{ return { x*rhs, y*rhs, z*rhs, w*rhs }; }
	inline constexpr quat operator / (float rhs) const
		{ return { x/rhs, y/rhs, z/rhs, w/rhs }; }

	// Arithmetic-assignment operators (quat with quat)
	inline quat& operator += (quat rhs)
		{ x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	inline quat& operator -= (quat rhs)
		{ x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	inline quat& operator *= (quat rhs)
		{ return *this = (*this * rhs); }

	// Arithmetic-assignment operators (quat with float)
	inline quat& operator += (float rhs)
		{ x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
	inline quat& operator -= (float rhs)
		{ x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
	inline quat& operator *= (float rhs)
		{ x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	inline quat& operator /= (float rhs)
		{ x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	// Rotate a vector by multiplying it //
	vec3 operator * (vec3 rhs)
	{
		vec3 qvec = { x, y, z };
		vec3 uv = vec3::cross(qvec, rhs);

		return rhs + (2.0f * ((uv * w) + vec3::cross(qvec, uv)));
	}

	// Unary Minus
	inline constexpr quat operator - () const
		{ return {-x, -y, -z, w}; }

	//// Member Functions
	inline constexpr quat conjugate() const
		{ return -*this; }

	inline constexpr bool is_zero() const
		{ return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 0.0f)); }

	inline float magnitude() const
		{ return sqrtf((x*x) + (y*y) + (z*z) + (w*w)); }

	inline float mag() const
		{ return magnitude(); }

	inline constexpr float magnitude_squared() const
		{ return ((x*x) + (y*y) + (z*z) + (w*w)); }

	inline constexpr float sqrmag() const
		{ return magnitude_squared(); }

	inline quat normalized() const
		{ return (*this / magnitude()); }

	inline void normalize()
		{ *this = normalized(); }

	inline vec3 to_euler()
	{
		return
		{
			atan2f(2.0f * (y * z + w * x), w * w - x * x - y * y + z * z),
			atan2f(2.0f * (x * y + w * z), w * w + x * x - y * y - z * z),
			asinf(-2.0f * (x * z - w * y)),
		};
	}

	inline vec4 to_axis_angle()
	{
		float temp = sqrtf(1 - w*w);
		return{ x / temp,
				y / temp,
				z / temp,
				2 * acosf(w) };
	}

	//// Static Functions
	static inline bool close_enough(quat lhs, quat rhs)
		{ return ((lhs - rhs).magnitude_squared() < 0.001f); }

	static inline constexpr float dot(quat a, quat b)
		{ return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w); }

	static inline quat euler(float pitch, float roll, float yaw)
	{
		quat result;

		float c1 = cosf(pitch * 0.5f);
		float c2 = cosf(roll * 0.5f);
		float c3 = cosf(yaw * 0.5f);
		float s1 = sinf(pitch * 0.5f);
		float s2 = sinf(roll * 0.5f);
		float s3 = sinf(yaw * 0.5f);

		result.w = (c1 * c2 * c3) + (s1 * s2 * s3);
		result.x = (s1 * c2 * c3) - (c1 * s2 * s3);
		result.y = (c1 * s2 * c3) + (s1 * c2 * s3);
		result.z = (c1 * c2 * s3) - (s1 * s2 * c3);

		return result;
	}

	static inline quat euler(vec3 angles)
		{ return euler(angles.x, angles.y, angles.z); }

	static inline quat axis_angle(vec3 axis, float angle)
	{
		float s = sinf(angle * 0.5f);
		float c = cosf(angle * 0.5f);
		axis.normalize();

		return
		{
			axis.x * s,
			axis.y * s,
			axis.z * s,
			c
		};
	}

	static inline quat axis_angle(vec4 in)
		{ return axis_angle(in.xyz, in.w); }

	static quat from_to_rotation(vec3 from, vec3 to)
	{
		vec3 xyz = vec3::cross(from, to);
		quat result = { xyz.x, xyz.y, xyz.z, sqrtf(from.magnitude_squared() * to.magnitude_squared()) + vec3::dot(from, to) };

		return result.normalized();
	}

	static inline quat lerp(quat from, quat to, float amount)
		{ return ((from * (1.0f - amount)) + (to * amount)).normalized(); }

	static inline quat rotation_between(quat a, quat b)
		{ return a * -b; }

	static inline float angle_between(quat a, quat b)
		{ return acosf(rotation_between(a, b).w); }

	static inline float quat_to_angle(quat in)
		{ return acosf(in.w); }

	static quat look_at(vec3 direction)
	{
		vec3 forward = direction.normalized();
		float dot = vec3::dot(VEC3_FORWARD, forward);

		if (fabsf(dot + 1.0f) < 0.00001f)
			{ return quat::axis_angle(VEC3_UP, PI); }
		if (fabsf(dot - 1.0f) < 0.00001f)
			{ return { 0, 0, 0, 1 }; }

		float angle = acosf(dot);
		vec3 axis = vec3::cross(VEC3_FORWARD, forward);
		return quat::axis_angle(axis, angle);
	}
};

constexpr quat QUAT_ZERO = { 0, 0, 0, 0 };
constexpr quat QUAT_IDENTITY = { 0, 0, 0, 1 };


// Arithmetic operator (float with quat)
inline constexpr quat operator + (float lhs, quat rhs)
	{ return { lhs+rhs.x, lhs+rhs.y, lhs+rhs.z, lhs+rhs.w }; }
inline constexpr quat operator - (float lhs, quat rhs)
	{ return { lhs-rhs.x, lhs-rhs.y, lhs-rhs.z, lhs-rhs.w }; }
inline constexpr quat operator * (float lhs, quat rhs)
	{ return { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z, lhs*rhs.w }; }
inline constexpr quat operator / (float lhs, quat rhs)
	{ return { lhs/rhs.x, lhs/rhs.y, lhs/rhs.z, lhs/rhs.w }; }

// Rotate a vector via "vec3 *= quat"
inline vec3& operator *= (vec3& lhs, quat rhs)
	{ return (lhs = rhs * lhs); }

} // namespace vmath
#endif
