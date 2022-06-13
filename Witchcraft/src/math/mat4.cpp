#include "mat4.h"
namespace vmath
{

	constexpr size_t INDEX_AT(size_t row, size_t col) { return (col * 4) + row; }
	


// Vector * Matrix
// Treat the vec4 as a row vector (1x4)


// Matrix * Matrix

// Inversion
mat4 mat4::inverted() const
{
	float invdet = 1.0f / determinant();

	return {
		(m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33) * invdet,
		(m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33) * invdet,
		(m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33) * invdet,
		(m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23) * invdet,
		(m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33) * invdet,
		(m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33) * invdet,
		(m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33) * invdet,
		(m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23) * invdet,
		(m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33) * invdet,
		(m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33) * invdet,
		(m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33) * invdet,
		(m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23) * invdet,
		(m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32) * invdet,
		(m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32) * invdet,
		(m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32) * invdet,
		(m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22) * invdet };
}


float mat4::determinant() const
{
	return 
		m03*m12*m21*m30 - m02*m13*m21*m30 - m03*m11*m22*m30 + m01*m13*m22*m30 +
		m02*m11*m23*m30 - m01*m12*m23*m30 - m03*m12*m20*m31 + m02*m13*m20*m31 +
		m03*m10*m22*m31 - m00*m13*m22*m31 - m02*m10*m23*m31 + m00*m12*m23*m31 +
		m03*m11*m20*m32 - m01*m13*m20*m32 - m03*m10*m21*m32 + m00*m13*m21*m32 +
		m01*m10*m23*m32 - m00*m11*m23*m32 - m02*m11*m20*m33 + m01*m12*m20*m33 +
		m02*m10*m21*m33 - m00*m12*m21*m33 - m01*m10*m22*m33 + m00*m11*m22*m33;
}


// Translation
mat4 mat4::translation(float x, float y, float z)
{
	return{ 1, 0, 0, x,
			0, 1, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1 };
}

// Scale
mat4 mat4::scale(float x, float y, float z)
{
	return{ x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1 };
}

// Rotation
mat4 mat4:: rotation(vec3 axis, float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);

	axis.normalize();

	return{	(axis.x*axis.x*(1-c))+c,			(axis.x*axis.y*(1-c))-(axis.z*s),	(axis.x*axis.z*(1-c))+(axis.y*s),	0,
			(axis.y*axis.x*(1-c))+(axis.z*s),	(axis.y*axis.y*(1-c))+c,			(axis.y*axis.z*(1-c))-(axis.x*s),	0,
			(axis.x*axis.z*(1-c))-(axis.y*s),	(axis.y*axis.z*(1-c))+(axis.x*s),	(axis.z*axis.z*(1-c))+c,			0,
			0, 0, 0, 1 };
}

mat4 mat4::rotation(quat q)
{
	return{ (q.w*q.w) + (q.x*q.x) - (q.y*q.y) - (q.z*q.z), 2 * ((q.x*q.y) - (q.w*q.z)), 2 * ((q.w*q.y) + (q.x*q.z)), 0,
			2 * ((q.x*q.y) + (q.w*q.z)), (q.w*q.w) - (q.x*q.x) + (q.y*q.y) - (q.z*q.z), 2 * ((q.y*q.z) - (q.w*q.x)), 0,
			2 * ((q.x*q.z) - (q.w*q.y)), 2 * ((q.w*q.x) + (q.y*q.z)), (q.w*q.w) - (q.x*q.x) - (q.y*q.y) + (q.z*q.z), 0,
			0, 0, 0, 1 };
}

// Look At
mat4 mat4::look_at(vec3 position, vec3 target, vec3 up)
{
	// Get the target vector (pointing from position to target. //
	vec3 f = (target - position).normalized();
	vec3 u = up.normalized();
	vec3 s = vec3::cross(f, u).normalized();
	u = vec3::cross(s, f);

	mat4 result = { s.x, s.y, s.z, 0,
					u.x, u.y, u.z, 0,
					-f.x, -f.y, -f.z, 0,
					0, 0, 0, 1 };

	return result * translation(-position);
}

// Projection
mat4 mat4::perspective(float fovy, float aspect, float znear, float zfar)
{
	// This is the canonical implementation of a perspective matrix.
	// When using this matrix, +y is up, no matter which API is running under the hood.

	float tan_half_fov = tanf(TO_RADIANS * (fovy * 0.5f));

	return{ 1 / (tan_half_fov * aspect), 0, 0, 0,
			0, 1 / tan_half_fov, 0, 0,
			0, 0, (znear + zfar) / (znear - zfar), (2*znear*zfar) / (znear - zfar),
			0, 0, -1, 0 };
}

mat4 mat4::perspective(float fovy, float aspect, float znear)
{
	// This modification of the perspective matrix uses a limit calculator
	// to see what each value would be if "zfar" were equal to infinity.
	// This sactifices a small amount of precision very close to znear,
	// in exchange for precision gains further away.

	// +y is still up.

	// We may have to scrap this cleverness, depending on how things go with the renderer.
	// Tricks like depth reconstruction and cascaded shadow maps might not play nicely with an infinite z-buffer.

	float f = 1 / tanf(TO_RADIANS * (fovy / 2));
	float e = 0.00000024f; // This bias is to ensure that points on the far plane aren't incorrectly culled.

	return{ f / aspect, 0, 0, 0,
			0, f, 0, 0,
			0, 0, e - 1, (e - 2)*znear,
			0, 0, -1, 0 };
}

mat4 mat4::ortho(float width, float height, float znear, float zfar)
{
	return{ 2 / width, 0, 0, -1,
			0, 2 / height, 0, -1,
			0, 0, -2 / (zfar - znear), -(zfar + znear) / (zfar - znear),
			0, 0, 0, 1 };
}

mat4 mat4::ortho(float left, float right, float bottom, float top, float near, float far)
{
	return{ 2 / (right - left), 0, 0, -(right + left) / (right - left),
			0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
			0, 0, -2 / (far - near), -(far + near) / (far - near),
			0, 0, 0, 1 };
}

vec3 mat4::extract_position()
{
	return columns[3].xyz;
}

vec3 mat4::extract_scale()
{
	return {columns[0].xyz.magnitude(), columns[1].xyz.magnitude(), columns[2].xyz.magnitude()};
}

quat mat4::extract_rotation()
{
	vec3 scale = extract_scale();
	mat4 copy = *this;
	copy.columns[0] /= scale.x; copy.columns[1] /= scale.y; copy.columns[2] /= scale.z;

	quat result;
	result.w = sqrtf(fmaxf(0, 1 + copy.m00 + copy.m11 + copy.m22)) / 2;
	result.x = sqrtf(fmaxf(0, 1 + copy.m00 - copy.m11 - copy.m22)) / 2;
	result.y = sqrtf(fmaxf(0, 1 - copy.m00 + copy.m11 - copy.m22)) / 2;
	result.z = sqrtf(fmaxf(0, 1 - copy.m00 - copy.m11 + copy.m22)) / 2;
	result.x = fabsf(result.x) * signof_or_zero(copy.m21 - copy.m12);
	result.y = fabsf(result.y) * signof_or_zero(copy.m02 - copy.m20);
	result.z = fabsf(result.z) * signof_or_zero(copy.m10 - copy.m01);
	return result;
}

} // namespace vmath