#ifndef HVH_WC_MATH_MAT4_H
#define HVH_WC_MATH_MAT4_H

#include "vec3.h"
#include "vec4.h"
#include "quat.h"

namespace vmath
{

struct mat4
{
	mat4() = default;
	mat4(const mat4&) = default;

	// In C, C++, and most other written languages,
	// defining a matrix as a series of values, read left-to-right,
	// creates a row-major matrix.
	// This constructor performs the transposition for us,
	// so we can store it in our column-major format.
	constexpr mat4(	float m00, float m01, float m02, float m03,
					float m10, float m11, float m12, float m13,
					float m20, float m21, float m22, float m23,
					float m30, float m31, float m32, float m33)
	:	m00(m00), m01(m01), m02(m02), m03(m03),
		m10(m10), m11(m11), m12(m12), m13(m13),
		m20(m20), m21(m21), m22(m22), m23(m23),
		m30(m30), m31(m31), m32(m32), m33(m33) {}


	//// Data storage
	union
	{
		float data[16];
		vec4 columns[4];
		struct { float  m00, m10, m20, m30,
						m01, m11, m21, m31,
						m02, m12, m22, m32,
						m03, m13, m23, m33; };
	};

	inline float& at(int row, int col)
		{ return data[(col*4)+row]; }
	//	{ return columns[col].data[row]; }

	inline float at(int row, int col) const
		{ return data[(col*4)+row]; }
	//	{ return columns[col].data[row]; }

	inline vec4 row(int which) const
	{
		return {columns[0].data[which], columns[1].data[which], columns[2].data[which], columns[3].data[which]};
	}

	inline vec4 col(int which) const
		{ return columns[which]; }


	// Matrix * Vector
	// Treat the vec4 as a column vector (4x1)
	inline vec4 operator * (vec4 rhs) const
	{
		vec4 result;

		for (int r = 0; r < 4; ++r)
		{
			result.data[r] = at(r, 0) * rhs.data[0];
			result.data[r] += at(r, 1) * rhs.data[1];
			result.data[r] += at(r, 2) * rhs.data[2];
			result.data[r] += at(r, 3) * rhs.data[3];
		}

		return result;
	}

	// Matrix * Matrix
	mat4 operator * (const mat4& rhs) const
	{
		mat4 result;

		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
			{
				// The element at the Rth row and Cth column of C is the dot product between the Rth row of A and the Cth column of B.
				result.at(r, c) = at(r, 0) * rhs.at(0, c);
				result.at(r, c) += at(r, 1) * rhs.at(1, c);
				result.at(r, c) += at(r, 2) * rhs.at(2, c);
				result.at(r, c) += at(r, 3) * rhs.at(3, c);
			}
		}

		return result;
	}

	// Matrix *= Matrix
	inline mat4& operator *= (const mat4& rhs)
		{ return (*this = *this * rhs); }

	// Equality testing
	// Sometimes useful to check if a matrix is an identity matrix
	inline bool operator == (const mat4& rhs)
	{
		for (int i = 0; i < 16; ++i)
			{ if (data[i] != rhs.data[i]) return false; }
		return true;
	}

	inline bool operator != (const mat4& rhs)
		{ return !(*this == rhs); }

	// Transposition is a quick function that doesn't rely on ordering,
	// So we inline it and do it here.
	inline mat4 transposed() const
	{
		mat4 result;
		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
			{
				result.at(r, c) = at(c, r);
			}
		}
		return result;
	}
	inline void transpose()
		{ *this = this->transposed(); }

	// Inversion
	mat4 inverted() const;
	inline void invert()
		{ *this = this->inverted(); }

	float determinant() const;

	// Translation
	static mat4 translation(float x, float y, float z);
	static inline mat4 translation(vec3 pos)
		{ return translation(pos.x, pos.y, pos.z); }

	// Scale
	static mat4 scale(float x, float y, float z);
	static inline mat4 scale(vec3 amounts)
		{ return scale(amounts.x, amounts.y, amounts.z); }

	// Rotation
	static mat4 rotation(vec3 axis, float angle);
	static mat4 rotation(quat q);

	// Look At
	static mat4 look_at(vec3 position, vec3 target, vec3 up = VEC3_UP);

	// Projection
	static mat4 perspective(float fovy, float aspect, float znear, float zfar);
	static mat4 perspective(float fovy, float aspect, float znear);

	static mat4 ortho(float width, float height, float znear, float zfar);
	static mat4 ortho(float left, float right, float bottom, float top, float near, float far);

	// Testing
	static bool RunUnitTests();

	// Extracting transformations
	vec3 extract_position();
	quat extract_rotation();
	vec3 extract_scale();
};

// Vector * Matrix
// Treat the vec4 as a row vector (1x4)
inline vec4 operator*(vec4 lhs, const mat4& rhs)
{
	vec4 result;

	for (int c = 0; c < 4; ++c)
	{
		result.data[c] = lhs.data[0] * rhs.at(0, c);
		result.data[c] += lhs.data[1] * rhs.at(1, c);
		result.data[c] += lhs.data[2] * rhs.at(2, c);
		result.data[c] += lhs.data[3] * rhs.at(3, c);
	}

	return result;
}

constexpr const mat4 MAT4_ZERO = {  0,0,0,0,
									0,0,0,0,
									0,0,0,0,
									0,0,0,0 };

constexpr const mat4 MAT4_IDENTITY = { 1,0,0,0,
									   0,1,0,0,
									   0,0,1,0,
									   0,0,0,1 };

} // namespace vmath
#endif
