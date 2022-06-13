#ifndef HVH_VMATH_VEC4_H
#define HVH_VMATH_VEC4_H

#include "vmath_includes.h"
#include "vec2.h"
#include "vec3.h"

namespace vmath
{

class vec4
{
public:
	//// Constructors
	vec4() = default;
	vec4(const vec4&) = default;
	constexpr vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	constexpr vec4(vec3 xyz, float w) : x(xyz.data[0]), y(xyz.data[1]), z(xyz.data[2]), w(w) {}
	constexpr vec4(float x, vec3 yzw) : x(x), y(yzw.data[0]), z(yzw.data[1]), w(yzw.data[2]) {}
	constexpr vec4(vec2 xy, vec2 zw) : x(xy.data[0]), y(xy.data[1]), z(zw.data[0]), w(zw.data[1]) {}
	constexpr vec4(vec2 xy, float z, float w) : x(xy.data[0]), y(xy.data[1]), z(z), w(w) {}
	constexpr vec4(float x, vec2 yz, float w) : x(x), y(yz.data[0]), b(yz.data[1]), w(w) {}
	constexpr vec4(float x, float y, vec2 zw) : x(x), y(y), b(zw.data[0]), a(zw.data[1]) {}

	//// Swizzles
private:

	template <typename T, int X, int Y, int Z, int W>
	struct SWIZZLE_READ_ONLY
	{
		inline operator vec4() const
		{
			T* self = (T*)this;
			return{ self->data[X], self->data[Y], self->data[Z], self->data[W] };
		}

		inline vec4 operator + (vec4 rhs)
			{ T* self = (T*)this; return{ self->data[X] + rhs.data[0], self->data[Y] + rhs.data[1], self->data[Z] + rhs.data[2], self->data[W] + rhs.data[3] }; }
		inline vec4 operator - (vec4 rhs)
			{ T* self = (T*)this; return{ self->data[X] - rhs.data[0], self->data[Y] - rhs.data[1], self->data[Z] - rhs.data[2], self->data[W] - rhs.data[3] }; }
		inline vec4 operator * (vec4 rhs)
			{ T* self = (T*)this; return{ self->data[X] * rhs.data[0], self->data[Y] * rhs.data[1], self->data[Z] * rhs.data[2], self->data[W] * rhs.data[3] }; }
		inline vec4 operator / (vec4 rhs)
			{ T* self = (T*)this; return{ self->data[X] / rhs.data[0], self->data[Y] / rhs.data[1], self->data[Z] / rhs.data[2], self->data[W] / rhs.data[3] }; }

		inline vec4 operator + (float rhs)
			{ T* self = (T*)this; return{ self->data[X] + rhs, self->data[Y] + rhs, self->data[Z] + rhs, self->data[W] + rhs }; }
		inline vec4 operator - (float rhs)
			{ T* self = (T*)this; return{ self->data[X] - rhs, self->data[Y] - rhs, self->data[Z] - rhs, self->data[W] / rhs }; }
		inline vec4 operator * (float rhs)
			{ T* self = (T*)this; return{ self->data[X] * rhs, self->data[Y] * rhs, self->data[Z] * rhs, self->data[W] * rhs }; }
		inline vec4 operator / (float rhs)
			{ T* self = (T*)this; return{ self->data[X] / rhs, self->data[Y] / rhs, self->data[Z] / rhs, self->data[W] / rhs }; }
	};

	template <typename T, int X, int Y, int Z, int W>
	struct SWIZZLE_READWRITE : public SWIZZLE_READ_ONLY<T, X, Y, Z, W>
	{
		static_assert((X != Y) && (X != Z) && (Y != Z) && (X != W) && (Y != W) && (Z != W), "Read+Write saibbling cannot be defined for multiple identical elements.");

		inline T& operator = (vec4 rhs)
		{
			T* self = (T*)this;
			self->data[X] = rhs.data[0];
			self->data[Y] = rhs.data[1];
			self->data[Z] = rhs.data[2];
			self->data[W] = rhs.data[3];
			return *self;
		}

		// Any non-const, non-static member functions need to be defined here.

		inline vec4& operator += (vec4 rhs)
			{ T* self = (T*)this; self->data[X] += rhs.data[0]; self->data[Y] += rhs.data[1]; self->data[Z] += rhs.data[2]; self->data[W] += rhs.data[3]; return *self; }
		inline vec4& operator -= (vec4 rhs)
			{ T* self = (T*)this; self->data[X] -= rhs.data[0]; self->data[Y] -= rhs.data[1]; self->data[Z] -= rhs.data[2]; self->data[W] -= rhs.data[3]; return *self; }
		inline vec4& operator *= (vec4 rhs)
			{ T* self = (T*)this; self->data[X] *= rhs.data[0]; self->data[Y] *= rhs.data[1]; self->data[Z] *= rhs.data[2]; self->data[W] *= rhs.data[3]; return *self; }
		inline vec4& operator /= (vec4 rhs)
			{ T* self = (T*)this; self->data[X] /= rhs.data[0]; self->data[Y] /= rhs.data[1]; self->data[Z] /= rhs.data[2]; self->data[W] /= rhs.data[3]; return *self; }

		inline vec4& operator += (float rhs)
			{ T* self = (T*)this; self->data[X] += rhs; self->data[Y] += rhs; self->data[Z] += rhs; self->data[W] += rhs; return *self; }
		inline vec4& operator -= (float rhs)
			{ T* self = (T*)this; self->data[X] -= rhs; self->data[Y] -= rhs; self->data[Z] -= rhs; self->data[W] += rhs; return *self; }
		inline vec4& operator *= (float rhs)
			{ T* self = (T*)this; self->data[X] *= rhs; self->data[Y] *= rhs; self->data[Z] *= rhs; self->data[W] += rhs; return *self; }
		inline vec4& operator /= (float rhs)
			{ T* self = (T*)this; self->data[X] /= rhs; self->data[Y] /= rhs; self->data[Z] /= rhs; self->data[W] += rhs; return *self; }

		inline void normalize()
		{
			T* self = (T*)this;
			vec4 temp = { self->data[X], self->data[Y], self->data[Z], self->data[W] };
			temp.normalize();
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2]; self->data[W] = temp.data[3];
		}

		inline void clamp(vec4 minimum, vec4 maximum)
		{
			T* self = (T*)this;
			vec4 temp = { self->data[X], self->data[Y], self->data[Z], self->data[W] };
			temp.clamp(minimum, maximum);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2]; self->data[W] = temp.data[3];
		}

		inline void clamp_magnitude(float max_magnitude)
		{
			T* self = (T*)this;
			vec4 temp = { self->data[X], self->data[Y], self->data[Z], self->data[W] };
			temp.clamp_magnitude(max_magnitude);
			self->data[X] = temp.data[0]; self->data[Y] = temp.data[1]; self->data[Z] = temp.data[2]; self->data[W] = temp.data[3];
		}

	};

public:
	//// Data storage

	union
	{
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		struct { float s, t, p, q; };
		float data[4];

		//// vec4 swizzles
		// xyzw
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 0> xxxx;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 1> xxxy;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 2> xxxz;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 3> xxxw;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 0> xxyx;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 1> xxyy;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 2> xxyz;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 3> xxyw;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 0> xxzx;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 1> xxzy;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 2> xxzz;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 3> xxzw;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 0> xxwx;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 1> xxwy;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 2> xxwz;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 3> xxww;

		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 0> xyxx;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 1> xyxy;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 2> xyxz;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 3> xyxw;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 0> xyyx;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 1> xyyy;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 2> xyyz;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 3> xyyw;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 0> xyzx;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 1> xyzy;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 2> xyzz;
		SWIZZLE_READWRITE<vec4, 0, 1, 2, 3> xyzw;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 0> xywx;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 1> xywy;
		SWIZZLE_READWRITE<vec4, 0, 1, 3, 2> xywz;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 3> xyww;

		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 0> xzxx;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 1> xzxy;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 2> xzxz;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 3> xzxw;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 0> xzyx;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 1> xzyy;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 2> xzyz;
		SWIZZLE_READWRITE<vec4, 0, 2, 1, 3> xzyw;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 0> xzzx;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 1> xzzy;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 2> xzzz;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 3> xzzw;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 0> xzwx;
		SWIZZLE_READWRITE<vec4, 0, 2, 3, 1> xzwy;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 2> xzwz;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 3> xzww;

		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 0> xwxx;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 1> xwxy;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 2> xwxz;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 3> xwxw;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 0> xwyx;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 1> xwyy;
		SWIZZLE_READWRITE<vec4, 0, 3, 1, 2> xwyz;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 3> xwyw;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 0> xwzx;
		SWIZZLE_READWRITE<vec4, 0, 3, 2, 1> xwzy;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 2> xwzz;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 3> xwzw;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 0> xwwx;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 1> xwwy;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 2> xwwz;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 3> xwww;

		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 0> yxxx;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 1> yxxy;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 2> yxxz;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 3> yxxw;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 0> yxyx;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 1> yxyy;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 2> yxyz;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 3> yxyw;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 0> yxzx;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 1> yxzy;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 2> yxzz;
		SWIZZLE_READWRITE<vec4, 1, 0, 2, 3> yxzw;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 0> yxwx;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 1> yxwy;
		SWIZZLE_READWRITE<vec4, 1, 0, 3, 2> yxwz;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 3> yxww;

		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 0> yyxx;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 1> yyxy;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 2> yyxz;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 3> yyxw;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 0> yyyx;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 1> yyyy;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 2> yyyz;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 3> yyyw;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 0> yyzx;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 1> yyzy;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 2> yyzz;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 3> yyzw;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 0> yywx;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 1> yywy;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 2> yywz;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 3> yyww;

		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 0> yzxx;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 1> yzxy;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 2> yzxz;
		SWIZZLE_READWRITE<vec4, 1, 2, 0, 3> yzxw;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 0> yzyx;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 1> yzyy;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 2> yzyz;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 3> yzyw;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 0> yzzx;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 1> yzzy;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 2> yzzz;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 3> yzzw;
		SWIZZLE_READWRITE<vec4, 1, 2, 3, 0> yzwx;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 1> yzwy;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 2> yzwz;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 3> yzww;

		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 0> ywxx;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 1> ywxy;
		SWIZZLE_READWRITE<vec4, 1, 3, 0, 2> ywxz;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 3> ywxw;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 0> ywyx;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 1> ywyy;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 2> ywyz;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 3> ywyw;
		SWIZZLE_READWRITE<vec4, 1, 3, 2, 0> ywzx;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 1> ywzy;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 2> ywzz;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 3> ywzw;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 0> ywwx;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 1> ywwy;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 2> ywwz;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 3> ywww;

		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 0> zxxx;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 1> zxxy;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 2> zxxz;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 3> zxxw;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 0> zxyx;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 1> zxyy;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 2> zxyz;
		SWIZZLE_READWRITE<vec4, 2, 0, 1, 3> zxyw;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 0> zxzx;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 1> zxzy;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 2> zxzz;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 3> zxzw;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 0> zxwx;
		SWIZZLE_READWRITE<vec4, 2, 0, 3, 1> zxwy;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 2> zxwz;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 3> zxww;

		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 0> zyxx;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 1> zyxy;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 2> zyxz;
		SWIZZLE_READWRITE<vec4, 2, 1, 0, 3> zyxw;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 0> zyyx;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 1> zyyy;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 2> zyyz;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 3> zyyw;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 0> zyzx;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 1> zyzy;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 2> zyzz;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 3> zyzw;
		SWIZZLE_READWRITE<vec4, 2, 1, 3, 0> zywx;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 1> zywy;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 2> zywz;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 3> zyww;

		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 0> zzxx;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 1> zzxy;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 2> zzxz;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 3> zzxw;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 0> zzyx;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 1> zzyy;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 2> zzyz;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 3> zzyw;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 0> zzzx;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 1> zzzy;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 2> zzzz;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 3> zzzw;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 0> zzwx;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 1> zzwy;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 2> zzwz;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 3> zzww;

		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 0> zwxx;
		SWIZZLE_READWRITE<vec4, 2, 3, 0, 1> zwxy;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 2> zwxz;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 3> zwxw;
		SWIZZLE_READWRITE<vec4, 2, 3, 1, 0> zwyx;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 1> zwyy;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 2> zwyz;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 3> zwyw;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 0> zwzx;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 1> zwzy;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 2> zwzz;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 3> zwzw;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 0> zwwx;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 1> zwwy;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 2> zwwz;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 3> zwww;

		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 0> wxxx;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 1> wxxy;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 2> wxxz;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 3> wxxw;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 0> wxyx;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 1> wxyy;
		SWIZZLE_READWRITE<vec4, 3, 0, 1, 2> wxyz;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 3> wxyw;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 0> wxzx;
		SWIZZLE_READWRITE<vec4, 3, 0, 2, 1> wxzy;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 2> wxzz;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 3> wxzw;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 0> wxwx;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 1> wxwy;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 2> wxwz;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 3> wxww;

		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 0> wyxx;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 1> wyxy;
		SWIZZLE_READWRITE<vec4, 3, 1, 0, 2> wyxz;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 3> wyxw;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 0> wyyx;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 1> wyyy;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 2> wyyz;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 3> wyyw;
		SWIZZLE_READWRITE<vec4, 3, 1, 2, 0> wyzx;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 1> wyzy;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 2> wyzz;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 3> wyzw;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 0> wywx;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 1> wywy;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 2> wywz;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 3> wyww;

		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 0> wzxx;
		SWIZZLE_READWRITE<vec4, 3, 2, 0, 1> wzxy;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 2> wzxz;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 3> wzxw;
		SWIZZLE_READWRITE<vec4, 3, 2, 1, 0> wzyx;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 1> wzyy;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 2> wzyz;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 3> wzyw;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 0> wzzx;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 1> wzzy;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 2> wzzz;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 3> wzzw;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 0> wzwx;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 1> wzwy;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 2> wzwz;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 3> wzww;

		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 0> wwxx;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 1> wwxy;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 2> wwxz;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 3> wwxw;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 0> wwyx;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 1> wwyy;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 2> wwyz;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 3> wwyw;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 0> wwzx;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 1> wwzy;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 2> wwzz;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 3> wwzw;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 0> wwwx;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 1> wwwy;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 2> wwwz;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 3> wwww;

		// rgba
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 0> rrrr;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 1> rrrg;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 2> rrrb;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 3> rrra;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 0> rrgr;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 1> rrgg;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 2> rrgb;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 3> rrga;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 0> rrbr;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 1> rrbg;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 2> rrbb;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 3> rrba;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 0> rrar;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 1> rrag;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 2> rrab;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 3> rraa;

		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 0> rgrr;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 1> rgrg;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 2> rgrb;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 3> rgra;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 0> rggr;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 1> rggg;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 2> rggb;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 3> rgga;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 0> rgbr;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 1> rgbg;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 2> rgbb;
		SWIZZLE_READWRITE<vec4, 0, 1, 2, 3> rgba;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 0> rgar;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 1> rgag;
		SWIZZLE_READWRITE<vec4, 0, 1, 3, 2> rgab;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 3> rgaa;

		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 0> rbrr;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 1> rbrg;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 2> rbrb;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 3> rbra;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 0> rbgr;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 1> rbgg;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 2> rbgb;
		SWIZZLE_READWRITE<vec4, 0, 2, 1, 3> rbga;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 0> rbbr;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 1> rbbg;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 2> rbbb;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 3> rbba;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 0> rbar;
		SWIZZLE_READWRITE<vec4, 0, 2, 3, 1> rbag;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 2> rbab;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 3> rbaa;

		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 0> rarr;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 1> rarg;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 2> rarb;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 3> rara;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 0> ragr;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 1> ragg;
		SWIZZLE_READWRITE<vec4, 0, 3, 1, 2> ragb;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 3> raga;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 0> rabr;
		SWIZZLE_READWRITE<vec4, 0, 3, 2, 1> rabg;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 2> rabb;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 3> raba;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 0> raar;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 1> raag;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 2> raab;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 3> raaa;

		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 0> grrr;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 1> grrg;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 2> grrb;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 3> grra;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 0> grgr;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 1> grgg;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 2> grgb;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 3> grga;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 0> grbr;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 1> grbg;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 2> grbb;
		SWIZZLE_READWRITE<vec4, 1, 0, 2, 3> grba;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 0> grar;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 1> grag;
		SWIZZLE_READWRITE<vec4, 1, 0, 3, 2> grab;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 3> graa;

		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 0> ggrr;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 1> ggrg;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 2> ggrb;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 3> ggra;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 0> gggr;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 1> gggg;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 2> gggb;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 3> ggga;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 0> ggbr;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 1> ggbg;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 2> ggbb;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 3> ggba;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 0> ggar;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 1> ggag;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 2> ggab;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 3> ggaa;

		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 0> gbrr;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 1> gbrg;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 2> gbrb;
		SWIZZLE_READWRITE<vec4, 1, 2, 0, 3> gbra;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 0> gbgr;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 1> gbgg;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 2> gbgb;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 3> gbga;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 0> gbbr;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 1> gbbg;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 2> gbbb;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 3> gbba;
		SWIZZLE_READWRITE<vec4, 1, 2, 3, 0> gbar;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 1> gbag;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 2> gbab;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 3> gbaa;

		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 0> garr;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 1> garg;
		SWIZZLE_READWRITE<vec4, 1, 3, 0, 2> garb;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 3> gara;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 0> gagr;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 1> gagg;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 2> gagb;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 3> gaga;
		SWIZZLE_READWRITE<vec4, 1, 3, 2, 0> gabr;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 1> gabg;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 2> gabb;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 3> gaba;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 0> gaar;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 1> gaag;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 2> gaab;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 3> gaaa;

		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 0> brrr;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 1> brrg;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 2> brrb;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 3> brra;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 0> brgr;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 1> brgg;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 2> brgb;
		SWIZZLE_READWRITE<vec4, 2, 0, 1, 3> brga;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 0> brbr;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 1> brbg;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 2> brbb;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 3> brba;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 0> brar;
		SWIZZLE_READWRITE<vec4, 2, 0, 3, 1> brag;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 2> brab;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 3> braa;

		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 0> bgrr;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 1> bgrg;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 2> bgrb;
		SWIZZLE_READWRITE<vec4, 2, 1, 0, 3> bgra;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 0> bggr;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 1> bggg;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 2> bggb;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 3> bgga;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 0> bgbr;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 1> bgbg;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 2> bgbb;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 3> bgba;
		SWIZZLE_READWRITE<vec4, 2, 1, 3, 0> bgar;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 1> bgag;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 2> bgab;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 3> bgaa;

		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 0> bbrr;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 1> bbrg;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 2> bbrb;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 3> bbra;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 0> bbgr;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 1> bbgg;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 2> bbgb;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 3> bbga;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 0> bbbr;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 1> bbbg;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 2> bbbb;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 3> bbba;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 0> bbar;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 1> bbag;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 2> bbab;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 3> bbaa;

		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 0> barr;
		SWIZZLE_READWRITE<vec4, 2, 3, 0, 1> barg;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 2> barb;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 3> bara;
		SWIZZLE_READWRITE<vec4, 2, 3, 1, 0> bagr;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 1> bagg;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 2> bagb;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 3> baga;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 0> babr;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 1> babg;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 2> babb;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 3> baba;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 0> baar;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 1> baag;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 2> baab;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 3> baaa;

		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 0> arrr;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 1> arrg;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 2> arrb;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 3> arra;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 0> argr;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 1> argg;
		SWIZZLE_READWRITE<vec4, 3, 0, 1, 2> argb;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 3> arga;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 0> arbr;
		SWIZZLE_READWRITE<vec4, 3, 0, 2, 1> arbg;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 2> arbb;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 3> arba;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 0> arar;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 1> arag;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 2> arab;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 3> araa;

		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 0> agrr;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 1> agrg;
		SWIZZLE_READWRITE<vec4, 3, 1, 0, 2> agrb;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 3> agra;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 0> aggr;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 1> aggg;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 2> aggb;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 3> agga;
		SWIZZLE_READWRITE<vec4, 3, 1, 2, 0> agbr;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 1> agbg;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 2> agbb;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 3> agba;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 0> agar;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 1> agag;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 2> agab;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 3> agaa;

		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 0> abrr;
		SWIZZLE_READWRITE<vec4, 3, 2, 0, 1> abrg;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 2> abrb;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 3> abra;
		SWIZZLE_READWRITE<vec4, 3, 2, 1, 0> abgr;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 1> abgg;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 2> abgb;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 3> abga;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 0> abbr;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 1> abbg;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 2> abbb;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 3> abba;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 0> abar;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 1> abag;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 2> abab;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 3> abaa;

		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 0> aarr;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 1> aarg;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 2> aarb;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 3> aara;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 0> aagr;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 1> aagg;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 2> aagb;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 3> aaga;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 0> aabr;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 1> aabg;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 2> aabb;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 3> aaba;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 0> aaar;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 1> aaag;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 2> aaab;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 3> aaaa;

		// stpq
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 0> ssss;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 1> ssst;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 2> sssp;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 0, 3> sssq;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 0> ssts;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 1> sstt;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 2> sstp;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 1, 3> sstq;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 0> ssps;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 1> sspt;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 2> sspp;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 2, 3> sspq;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 0> ssqs;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 1> ssqt;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 2> ssqp;
		SWIZZLE_READ_ONLY<vec4, 0, 0, 3, 3> ssqq;

		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 0> stss;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 1> stst;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 2> stsp;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 0, 3> stsq;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 0> stts;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 1> sttt;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 2> sttp;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 1, 3> sttq;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 0> stps;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 1> stpt;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 2, 2> stpp;
		SWIZZLE_READWRITE<vec4, 0, 1, 2, 3> stpq;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 0> stqs;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 1> stqt;
		SWIZZLE_READWRITE<vec4, 0, 1, 3, 2> stqp;
		SWIZZLE_READ_ONLY<vec4, 0, 1, 3, 3> stqq;

		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 0> spss;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 1> spst;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 2> spsp;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 0, 3> spsq;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 0> spts;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 1> sptt;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 1, 2> sptp;
		SWIZZLE_READWRITE<vec4, 0, 2, 1, 3> sptq;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 0> spps;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 1> sppt;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 2> sppp;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 2, 3> sppq;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 0> spqs;
		SWIZZLE_READWRITE<vec4, 0, 2, 3, 1> spqt;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 2> spqp;
		SWIZZLE_READ_ONLY<vec4, 0, 2, 3, 3> spqq;

		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 0> sqss;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 1> sqst;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 2> sqsp;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 0, 3> sqsq;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 0> sqts;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 1> sqtt;
		SWIZZLE_READWRITE<vec4, 0, 3, 1, 2> sqtp;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 1, 3> sqtq;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 0> sqps;
		SWIZZLE_READWRITE<vec4, 0, 3, 2, 1> sqpt;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 2> sqpp;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 2, 3> sqpq;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 0> sqqs;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 1> sqqt;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 2> sqqp;
		SWIZZLE_READ_ONLY<vec4, 0, 3, 3, 3> sqqq;

		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 0> tsss;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 1> tsst;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 2> tssp;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 0, 3> tssq;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 0> tsts;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 1> tstt;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 2> tstp;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 1, 3> tstq;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 0> tsps;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 1> tspt;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 2, 2> tspp;
		SWIZZLE_READWRITE<vec4, 1, 0, 2, 3> tspq;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 0> tsqs;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 1> tsqt;
		SWIZZLE_READWRITE<vec4, 1, 0, 3, 2> tsqp;
		SWIZZLE_READ_ONLY<vec4, 1, 0, 3, 3> tsqq;

		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 0> ttss;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 1> ttst;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 2> ttsp;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 0, 3> ttsq;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 0> ttts;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 1> tttt;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 2> tttp;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 1, 3> tttq;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 0> ttps;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 1> ttpt;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 2> ttpp;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 2, 3> ttpq;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 0> ttqs;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 1> ttqt;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 2> ttqp;
		SWIZZLE_READ_ONLY<vec4, 1, 1, 3, 3> ttqq;

		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 0> tpss;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 1> tpst;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 0, 2> tpsp;
		SWIZZLE_READWRITE<vec4, 1, 2, 0, 3> tpsq;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 0> tpts;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 1> tptt;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 2> tptp;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 1, 3> tptq;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 0> tpps;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 1> tppt;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 2> tppp;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 2, 3> tppq;
		SWIZZLE_READWRITE<vec4, 1, 2, 3, 0> tpqs;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 1> tpqt;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 2> tpqp;
		SWIZZLE_READ_ONLY<vec4, 1, 2, 3, 3> tpqq;

		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 0> tqss;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 1> tqst;
		SWIZZLE_READWRITE<vec4, 1, 3, 0, 2> tqsp;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 0, 3> tqsq;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 0> tqts;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 1> tqtt;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 2> tqtp;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 1, 3> tqtq;
		SWIZZLE_READWRITE<vec4, 1, 3, 2, 0> tqps;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 1> tqpt;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 2> tqpp;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 2, 3> tqpq;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 0> tqqs;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 1> tqqt;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 2> tqqp;
		SWIZZLE_READ_ONLY<vec4, 1, 3, 3, 3> tqqq;

		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 0> psss;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 1> psst;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 2> pssp;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 0, 3> pssq;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 0> psts;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 1> pstt;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 1, 2> pstp;
		SWIZZLE_READWRITE<vec4, 2, 0, 1, 3> pstq;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 0> psps;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 1> pspt;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 2> pspp;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 2, 3> pspq;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 0> psqs;
		SWIZZLE_READWRITE<vec4, 2, 0, 3, 1> psqt;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 2> psqp;
		SWIZZLE_READ_ONLY<vec4, 2, 0, 3, 3> psqq;

		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 0> ptss;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 1> ptst;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 0, 2> ptsp;
		SWIZZLE_READWRITE<vec4, 2, 1, 0, 3> ptsq;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 0> ptts;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 1> pttt;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 2> pttp;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 1, 3> pttq;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 0> ptps;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 1> ptpt;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 2> ptpp;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 2, 3> ptpq;
		SWIZZLE_READWRITE<vec4, 2, 1, 3, 0> ptqs;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 1> ptqt;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 2> ptqp;
		SWIZZLE_READ_ONLY<vec4, 2, 1, 3, 3> ptqq;

		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 0> ppss;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 1> ppst;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 2> ppsp;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 0, 3> ppsq;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 0> ppts;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 1> pptt;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 2> pptp;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 1, 3> pptq;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 0> ppps;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 1> pppt;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 2> pppp;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 2, 3> pppq;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 0> ppqs;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 1> ppqt;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 2> ppqp;
		SWIZZLE_READ_ONLY<vec4, 2, 2, 3, 3> ppqq;

		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 0> pqss;
		SWIZZLE_READWRITE<vec4, 2, 3, 0, 1> pqst;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 2> pqsp;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 0, 3> pqsq;
		SWIZZLE_READWRITE<vec4, 2, 3, 1, 0> pqts;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 1> pqtt;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 2> pqtp;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 1, 3> pqtq;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 0> pqps;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 1> pqpt;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 2> pqpp;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 2, 3> pqpq;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 0> pqqs;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 1> pqqt;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 2> pqqp;
		SWIZZLE_READ_ONLY<vec4, 2, 3, 3, 3> pqqq;

		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 0> qsss;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 1> qsst;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 2> qssp;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 0, 3> qssq;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 0> qsts;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 1> qstt;
		SWIZZLE_READWRITE<vec4, 3, 0, 1, 2> qstp;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 1, 3> qstq;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 0> qsps;
		SWIZZLE_READWRITE<vec4, 3, 0, 2, 1> qspt;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 2> qspp;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 2, 3> qspq;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 0> qsqs;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 1> qsqt;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 2> qsqp;
		SWIZZLE_READ_ONLY<vec4, 3, 0, 3, 3> qsqq;

		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 0> qtss;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 1> qtst;
		SWIZZLE_READWRITE<vec4, 3, 1, 0, 2> qtsp;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 0, 3> qtsq;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 0> qtts;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 1> qttt;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 2> qttp;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 1, 3> qttq;
		SWIZZLE_READWRITE<vec4, 3, 1, 2, 0> qtps;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 1> qtpt;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 2> qtpp;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 2, 3> qtpq;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 0> qtqs;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 1> qtqt;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 2> qtqp;
		SWIZZLE_READ_ONLY<vec4, 3, 1, 3, 3> qtqq;

		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 0> qpss;
		SWIZZLE_READWRITE<vec4, 3, 2, 0, 1> qpst;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 2> qpsp;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 0, 3> qpsq;
		SWIZZLE_READWRITE<vec4, 3, 2, 1, 0> qpts;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 1> qptt;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 2> qptp;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 1, 3> qptq;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 0> qpps;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 1> qppt;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 2> qppp;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 2, 3> qppq;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 0> qpqs;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 1> qpqt;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 2> qpqp;
		SWIZZLE_READ_ONLY<vec4, 3, 2, 3, 3> qpqq;

		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 0> qqss;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 1> qqst;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 2> qqsp;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 0, 3> qqsq;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 0> qqts;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 1> qqtt;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 2> qqtp;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 1, 3> qqtq;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 0> qqps;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 1> qqpt;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 2> qqpp;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 2, 3> qqpq;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 0> qqqs;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 1> qqqt;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 2> qqqp;
		SWIZZLE_READ_ONLY<vec4, 3, 3, 3, 3> qqqq;

		//// vec3 swizzles
		// xyzw
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 0> xxx;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 1> xxy;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 2> xxz;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 3> xxw;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 1, 0> xyx;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 1, 1> xyy;
		vec3::SWIZZLE_READWRITE<vec4, 0, 1, 2> xyz;
		vec3::SWIZZLE_READWRITE<vec4, 0, 1, 3> xyw;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 2, 0> xzx;
		vec3::SWIZZLE_READWRITE<vec4, 0, 2, 1> xzy;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 2, 2> xzz;
		vec3::SWIZZLE_READWRITE<vec4, 0, 2, 3> xzw;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 3, 0> xwx;
		vec3::SWIZZLE_READWRITE<vec4, 0, 3, 1> xwy;
		vec3::SWIZZLE_READWRITE<vec4, 0, 3, 2> xwz;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 3, 3> xww;

		vec3::SWIZZLE_READ_ONLY<vec4, 1, 0, 0> yxx;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 0, 1> yxy;
		vec3::SWIZZLE_READWRITE<vec4, 1, 0, 2> yxz;
		vec3::SWIZZLE_READWRITE<vec4, 1, 0, 3> yxw;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 0> yyx;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 1> yyy;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 2> yyz;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 3> yyw;
		vec3::SWIZZLE_READWRITE<vec4, 1, 2, 0> yzx;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 2, 1> yzy;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 2, 2> yzz;
		vec3::SWIZZLE_READWRITE<vec4, 1, 2, 3> yzw;
		vec3::SWIZZLE_READWRITE<vec4, 1, 3, 0> ywx;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 3, 1> ywy;
		vec3::SWIZZLE_READWRITE<vec4, 1, 3, 2> ywz;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 3, 3> yww;

		vec3::SWIZZLE_READ_ONLY<vec4, 2, 0, 0> zxx;
		vec3::SWIZZLE_READWRITE<vec4, 2, 0, 1> zxy;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 0, 2> zxz;
		vec3::SWIZZLE_READWRITE<vec4, 2, 0, 3> zxw;
		vec3::SWIZZLE_READWRITE<vec4, 2, 1, 0> zyx;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 1, 1> zyy;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 1, 2> zyz;
		vec3::SWIZZLE_READWRITE<vec4, 2, 1, 3> zyw;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 0> zzx;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 1> zzy;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 2> zzz;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 3> zzw;
		vec3::SWIZZLE_READWRITE<vec4, 2, 3, 0> zwx;
		vec3::SWIZZLE_READWRITE<vec4, 2, 3, 1> zwy;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 3, 2> zwz;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 3, 3> zww;

		vec3::SWIZZLE_READ_ONLY<vec4, 3, 0, 0> wxx;
		vec3::SWIZZLE_READWRITE<vec4, 3, 0, 1> wxy;
		vec3::SWIZZLE_READWRITE<vec4, 3, 0, 2> wxz;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 0, 3> wxw;
		vec3::SWIZZLE_READWRITE<vec4, 3, 1, 0> wyx;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 1, 1> wyy;
		vec3::SWIZZLE_READWRITE<vec4, 3, 1, 2> wyz;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 1, 3> wyw;
		vec3::SWIZZLE_READWRITE<vec4, 3, 2, 0> wzx;
		vec3::SWIZZLE_READWRITE<vec4, 3, 2, 1> wzy;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 2, 2> wzz;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 2, 3> wzw;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 0> wwx;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 1> wwy;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 2> wwz;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 3> www;

		// rgba
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 0> rrr;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 1> rrg;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 2> rrb;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 3> rra;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 1, 0> rgr;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 1, 1> rgg;
		vec3::SWIZZLE_READWRITE<vec4, 0, 1, 2> rgb;
		vec3::SWIZZLE_READWRITE<vec4, 0, 1, 3> rga;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 2, 0> rbr;
		vec3::SWIZZLE_READWRITE<vec4, 0, 2, 1> rbg;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 2, 2> rbb;
		vec3::SWIZZLE_READWRITE<vec4, 0, 2, 3> rba;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 3, 0> rar;
		vec3::SWIZZLE_READWRITE<vec4, 0, 3, 1> rag;
		vec3::SWIZZLE_READWRITE<vec4, 0, 3, 2> rab;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 3, 3> raa;

		vec3::SWIZZLE_READ_ONLY<vec4, 1, 0, 0> grr;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 0, 1> grg;
		vec3::SWIZZLE_READWRITE<vec4, 1, 0, 2> grb;
		vec3::SWIZZLE_READWRITE<vec4, 1, 0, 3> gra;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 0> ggr;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 1> ggg;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 2> ggb;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 3> gga;
		vec3::SWIZZLE_READWRITE<vec4, 1, 2, 0> gbr;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 2, 1> gbg;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 2, 2> gbb;
		vec3::SWIZZLE_READWRITE<vec4, 1, 2, 3> gba;
		vec3::SWIZZLE_READWRITE<vec4, 1, 3, 0> gar;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 3, 1> gag;
		vec3::SWIZZLE_READWRITE<vec4, 1, 3, 2> gab;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 3, 3> gaa;

		vec3::SWIZZLE_READ_ONLY<vec4, 2, 0, 0> brr;
		vec3::SWIZZLE_READWRITE<vec4, 2, 0, 1> brg;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 0, 2> brb;
		vec3::SWIZZLE_READWRITE<vec4, 2, 0, 3> bra;
		vec3::SWIZZLE_READWRITE<vec4, 2, 1, 0> bgr;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 1, 1> bgg;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 1, 2> bgb;
		vec3::SWIZZLE_READWRITE<vec4, 2, 1, 3> bga;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 0> bbr;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 1> bbg;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 2> bbb;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 3> bba;
		vec3::SWIZZLE_READWRITE<vec4, 2, 3, 0> bar;
		vec3::SWIZZLE_READWRITE<vec4, 2, 3, 1> bag;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 3, 2> bab;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 3, 3> baa;

		vec3::SWIZZLE_READ_ONLY<vec4, 3, 0, 0> arr;
		vec3::SWIZZLE_READWRITE<vec4, 3, 0, 1> arg;
		vec3::SWIZZLE_READWRITE<vec4, 3, 0, 2> arb;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 0, 3> ara;
		vec3::SWIZZLE_READWRITE<vec4, 3, 1, 0> agr;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 1, 1> agg;
		vec3::SWIZZLE_READWRITE<vec4, 3, 1, 2> agb;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 1, 3> aga;
		vec3::SWIZZLE_READWRITE<vec4, 3, 2, 0> abr;
		vec3::SWIZZLE_READWRITE<vec4, 3, 2, 1> abg;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 2, 2> abb;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 2, 3> aba;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 0> aar;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 1> aag;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 2> aab;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 3> aaa;

		// stpq
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 0> sss;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 1> sst;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 2> ssp;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 0, 3> ssq;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 1, 0> sts;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 1, 1> stt;
		vec3::SWIZZLE_READWRITE<vec4, 0, 1, 2> stp;
		vec3::SWIZZLE_READWRITE<vec4, 0, 1, 3> stq;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 2, 0> sps;
		vec3::SWIZZLE_READWRITE<vec4, 0, 2, 1> spt;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 2, 2> spp;
		vec3::SWIZZLE_READWRITE<vec4, 0, 2, 3> spq;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 3, 0> sqs;
		vec3::SWIZZLE_READWRITE<vec4, 0, 3, 1> sqt;
		vec3::SWIZZLE_READWRITE<vec4, 0, 3, 2> sqp;
		vec3::SWIZZLE_READ_ONLY<vec4, 0, 3, 3> sqq;

		vec3::SWIZZLE_READ_ONLY<vec4, 1, 0, 0> tss;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 0, 1> tst;
		vec3::SWIZZLE_READWRITE<vec4, 1, 0, 2> tsp;
		vec3::SWIZZLE_READWRITE<vec4, 1, 0, 3> tsq;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 0> tts;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 1> ttt;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 2> ttp;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 1, 3> ttq;
		vec3::SWIZZLE_READWRITE<vec4, 1, 2, 0> tps;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 2, 1> tpt;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 2, 2> tpp;
		vec3::SWIZZLE_READWRITE<vec4, 1, 2, 3> tpq;
		vec3::SWIZZLE_READWRITE<vec4, 1, 3, 0> tqs;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 3, 1> tqt;
		vec3::SWIZZLE_READWRITE<vec4, 1, 3, 2> tqp;
		vec3::SWIZZLE_READ_ONLY<vec4, 1, 3, 3> tqq;

		vec3::SWIZZLE_READ_ONLY<vec4, 2, 0, 0> pss;
		vec3::SWIZZLE_READWRITE<vec4, 2, 0, 1> pst;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 0, 2> psp;
		vec3::SWIZZLE_READWRITE<vec4, 2, 0, 3> psq;
		vec3::SWIZZLE_READWRITE<vec4, 2, 1, 0> pts;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 1, 1> ptt;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 1, 2> ptp;
		vec3::SWIZZLE_READWRITE<vec4, 2, 1, 3> ptq;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 0> pps;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 1> ppt;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 2> ppp;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 2, 3> ppq;
		vec3::SWIZZLE_READWRITE<vec4, 2, 3, 0> pqs;
		vec3::SWIZZLE_READWRITE<vec4, 2, 3, 1> pqt;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 3, 2> pqp;
		vec3::SWIZZLE_READ_ONLY<vec4, 2, 3, 3> pqq;

		vec3::SWIZZLE_READ_ONLY<vec4, 3, 0, 0> qss;
		vec3::SWIZZLE_READWRITE<vec4, 3, 0, 1> qst;
		vec3::SWIZZLE_READWRITE<vec4, 3, 0, 2> qsp;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 0, 3> qsq;
		vec3::SWIZZLE_READWRITE<vec4, 3, 1, 0> qts;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 1, 1> qtt;
		vec3::SWIZZLE_READWRITE<vec4, 3, 1, 2> qtp;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 1, 3> qtq;
		vec3::SWIZZLE_READWRITE<vec4, 3, 2, 0> qps;
		vec3::SWIZZLE_READWRITE<vec4, 3, 2, 1> qpt;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 2, 2> qpp;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 2, 3> qpq;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 0> qqs;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 1> qqt;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 2> qqp;
		vec3::SWIZZLE_READ_ONLY<vec4, 3, 3, 3> qqq;

		//// vec2 swizzles
		// xyzw
		vec2::SWIZZLE_READ_ONLY<vec4, 0, 0> xx;
		vec2::SWIZZLE_READWRITE<vec4, 0, 1> xy;
		vec2::SWIZZLE_READWRITE<vec4, 0, 2> xz;
		vec2::SWIZZLE_READWRITE<vec4, 0, 3> xw;
		vec2::SWIZZLE_READWRITE<vec4, 1, 0> yx;
		vec2::SWIZZLE_READ_ONLY<vec4, 1, 1> yy;
		vec2::SWIZZLE_READWRITE<vec4, 1, 2> yz;
		vec2::SWIZZLE_READWRITE<vec4, 1, 3> yw;
		vec2::SWIZZLE_READWRITE<vec4, 2, 0> zx;
		vec2::SWIZZLE_READWRITE<vec4, 2, 1> zy;
		vec2::SWIZZLE_READ_ONLY<vec4, 2, 2> zz;
		vec2::SWIZZLE_READWRITE<vec4, 2, 3> zw;
		vec2::SWIZZLE_READWRITE<vec4, 3, 0> wx;
		vec2::SWIZZLE_READWRITE<vec4, 3, 1> wy;
		vec2::SWIZZLE_READWRITE<vec4, 3, 2> wz;
		vec2::SWIZZLE_READ_ONLY<vec4, 3, 3> ww;

		// rgba
		vec2::SWIZZLE_READ_ONLY<vec4, 0, 0> rr;
		vec2::SWIZZLE_READWRITE<vec4, 0, 1> rg;
		vec2::SWIZZLE_READWRITE<vec4, 0, 2> rb;
		vec2::SWIZZLE_READWRITE<vec4, 0, 3> ra;
		vec2::SWIZZLE_READWRITE<vec4, 1, 0> gr;
		vec2::SWIZZLE_READ_ONLY<vec4, 1, 1> gg;
		vec2::SWIZZLE_READWRITE<vec4, 1, 2> gb;
		vec2::SWIZZLE_READWRITE<vec4, 1, 3> ga;
		vec2::SWIZZLE_READWRITE<vec4, 2, 0> br;
		vec2::SWIZZLE_READWRITE<vec4, 2, 1> bg;
		vec2::SWIZZLE_READ_ONLY<vec4, 2, 2> bb;
		vec2::SWIZZLE_READWRITE<vec4, 2, 3> ba;
		vec2::SWIZZLE_READWRITE<vec4, 3, 0> ar;
		vec2::SWIZZLE_READWRITE<vec4, 3, 1> ag;
		vec2::SWIZZLE_READWRITE<vec4, 3, 2> ab;
		vec2::SWIZZLE_READ_ONLY<vec4, 3, 3> aa;

		// stpq
		vec2::SWIZZLE_READ_ONLY<vec4, 0, 0> ss;
		vec2::SWIZZLE_READWRITE<vec4, 0, 1> st;
		vec2::SWIZZLE_READWRITE<vec4, 0, 2> sp;
		vec2::SWIZZLE_READWRITE<vec4, 0, 3> sq;
		vec2::SWIZZLE_READWRITE<vec4, 1, 0> ts;
		vec2::SWIZZLE_READ_ONLY<vec4, 1, 1> tt;
		vec2::SWIZZLE_READWRITE<vec4, 1, 2> tp;
		vec2::SWIZZLE_READWRITE<vec4, 1, 3> tq;
		vec2::SWIZZLE_READWRITE<vec4, 2, 0> ps;
		vec2::SWIZZLE_READWRITE<vec4, 2, 1> pt;
		vec2::SWIZZLE_READ_ONLY<vec4, 2, 2> pp;
		vec2::SWIZZLE_READWRITE<vec4, 2, 3> pq;
		vec2::SWIZZLE_READWRITE<vec4, 3, 0> qs;
		vec2::SWIZZLE_READWRITE<vec4, 3, 1> qt;
		vec2::SWIZZLE_READWRITE<vec4, 3, 2> qp;
		vec2::SWIZZLE_READ_ONLY<vec4, 3, 3> qq;
	};

	// Equality test.  Really only useful for checking if a vector is (still) equal to some constant, such as zero.
	inline constexpr bool operator == (vec4 rhs) const
		{ return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w)); }
	inline constexpr bool operator != (vec4 rhs) const
		{ return ((x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w)); }

	// Arithmetic operators (vector with vector)
	inline constexpr vec4 operator + (vec4 rhs) const
		{ return { x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w }; }
	inline constexpr vec4 operator - (vec4 rhs) const
		{ return { x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w }; }
	inline constexpr vec4 operator * (vec4 rhs) const
		{ return { x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w }; }
	inline constexpr vec4 operator / (vec4 rhs) const
		{ return { x/rhs.x, y/rhs.y, z/rhs.z, w/rhs.w }; }

	// Arithmetic operators (vector with float)
	inline constexpr vec4 operator + (float rhs) const
		{ return { x+rhs, y+rhs, z+rhs, w+rhs }; }
	inline constexpr vec4 operator - (float rhs) const
		{ return { x-rhs, y-rhs, z-rhs, w-rhs }; }
	inline constexpr vec4 operator * (float rhs) const
		{ return { x*rhs, y*rhs, z*rhs, w*rhs }; }
	inline constexpr vec4 operator / (float rhs) const
		{ return { x/rhs, y/rhs, z/rhs, w/rhs }; }

	// Arithmetic operator (float aith vector)
	inline constexpr friend vec4 operator + (float lhs, vec4 rhs)
		{ return { lhs+rhs.x, lhs+rhs.y, lhs+rhs.z, lhs+rhs.w }; }
	inline constexpr friend vec4 operator - (float lhs, vec4 rhs)
		{ return { lhs-rhs.x, lhs-rhs.y, lhs-rhs.z, lhs-rhs.w }; }
	inline constexpr friend vec4 operator * (float lhs, vec4 rhs)
		{ return { lhs*rhs.x, lhs*rhs.y, lhs*rhs.z, lhs*rhs.w }; }
	inline constexpr friend vec4 operator / (float lhs, vec4 rhs)
		{ return { lhs/rhs.x, lhs/rhs.y, lhs/rhs.z, lhs/rhs.w }; }

	// Arithmetic-assignment operators (vector aith vector)
	inline vec4& operator += (vec4 rhs)
		{ x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	inline vec4& operator -= (vec4 rhs)
		{ x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	inline vec4& operator *= (vec4 rhs)
		{ x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	inline vec4& operator /= (vec4 rhs)
		{ x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

	// Arithmetic-assignment operators (vector aith float)
	inline vec4& operator += (float rhs)
		{ x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
	inline vec4& operator -= (float rhs)
		{ x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
	inline vec4& operator *= (float rhs)
		{ x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	inline vec4& operator /= (float rhs)
		{ x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	// Negation operator
	inline constexpr vec4 operator - () const
		{ return { -x, -y, -z, -w }; }

	//// Member functions

	inline constexpr bool is_zero() const
		{ return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f) && (w == 0.0f)); }

	inline constexpr float sum() const
		{ return x + y + z + w; }

	inline float magnitude() const
		{ return sqrtf((x*x) + (y*y) + (z*z) + (w*w)); }

	inline float mag() const
		{ return magnitude(); }

	inline float magnitude_squared() const
		{ return (x*x) + (y*y) + (z*z) + (w*w); }

	inline float sqrmag() const
		{ return magnitude_squared(); }

	inline vec4 normalized() const
		{ return *this / magnitude(); }

	inline void normalize()
		{ *this = normalized(); }

	static inline bool close_enough(vec4 lhs, vec4 rhs)
		{ return ((lhs - rhs).magnitude_squared() < 0.001f); }

	static inline vec4 minimum(vec4 lhs, vec4 rhs)
		{ return { fminf(lhs.x, rhs.x), fminf(lhs.y, rhs.y), fminf(lhs.z, rhs.z), fminf(lhs.w, rhs.w) }; }

	static inline vec4 maximum(vec4 lhs, vec4 rhs)
		{ return { fmaxf(lhs.x, rhs.x), fmaxf(lhs.y, rhs.y), fmaxf(lhs.z, rhs.z), fmaxf(lhs.w, rhs.w) }; }

	inline vec4 clamped(vec4 lower, vec4 upper) const
		{ return minimum(upper, maximum(lower, *this)); }

	inline void clamp(vec4 lower, vec4 upper)
		{ *this = clamped(lower, upper); }

	inline vec4 clamped_magnitude(float max_magnitude) const
	{
		float mag = magnitude();
		if (mag <= max_magnitude)
			return *this;
		else
			return this->normalized() * max_magnitude;
	}

	inline void clamp_magnitude(float max_magnitude)
		{ *this = clamped_magnitude(max_magnitude); }

	static inline float distance(vec4 a, vec4 b)
		{ return (a-b).magnitude(); }

	static inline float distance_squared(vec4 a, vec4 b)
		{ return (a-b).magnitude_squared(); }

	inline vec4 moved_towards(vec4 target, float max_dist) const
	{
		if (distance(*this, target) <= max_dist)
			return target;
		else
			return *this + ((target - *this).normalized() * max_dist);
	}

	inline void move_towards(vec4 target, float max_dist)
		{ *this = moved_towards(target, max_dist); }

	static inline constexpr float dot(vec4 a, vec4 b)
		{ return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w); }

	// I got the implementation for this from Unity.  I honestly don't really know what the hell is going on here.
	static inline vec4 smooth_damp(const vec4 from, const vec4 to, vec4& velocity, float smooth_time, float delta_time, float max_speed)
	{
		if (smooth_time <= 0.0f)
			smooth_time = 0.0001f;
		float num = 2 / smooth_time;
		float num2 = num * delta_time;
		float num3 = 1 / (1 + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2);
		vec4 num4 = from - to;
		vec4 num5 = to;
		float max_dist = max_speed * smooth_time;
		num4.clamp_magnitude(max_dist);
		num5 = from - num4;
		vec4 num7 = ((num4 * num) + velocity) * delta_time;
		velocity = (velocity - num * num7) * num3;
		vec4 num8 = to + (num4 + num7) * num3;
		if (((num5 - from).magnitude() > 0) == (num8.magnitude() > num5.magnitude()))
		{
			num8 = num5;
			velocity = (num8 - num5) / delta_time;
		}

		return num8;
	}
};

constexpr vec4 VEC4_ZERO = { 0, 0, 0, 0 };
constexpr vec4 VEC4_ONE = { 1, 1, 1, 1 };

} // namespace vmath
#endif
