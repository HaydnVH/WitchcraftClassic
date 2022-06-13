#ifndef HVH_WC_GRAPHICS_VERTEX_H
#define HVH_WC_GRAPHICS_VERTEX_H

#include "math/vmath.h"

#include <stdint.h>

enum VertexFormatFlags
{
	VF_POSITION = 1 << 0,
	VF_SURFACE = 1 << 1,
	VF_SKIN = 1 << 2,
	VF_SKIN_LARGE = 1 << 3,
	VF_2D = 1 << 4,
};


struct VertexPosition
{
	float x, y, z;

	VertexPosition& operator = (vmath::vec3 rhs)
		{ x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

	vmath::vec3 to_vec3()
		{ return {x, y, z}; }

}; // 12 bytes; can possibly reduce to 8 by using ushorts and scaling the mesh using its matrix.

struct VertexSurface
{
	vmath::half s, t;
	int8_t nx, ny, nz, shading;
	int8_t tx, ty, tz, bs;
	
	vmath::vec2 texcoord_to_vec2()
		{ return {(float)s, (float)t}; }

	void vec2_to_texcoord(vmath::vec2 in)
		{ s = in.s; t = in.t; }

	vmath::vec3 normal_to_vec3()
		{ return {(float)nx / CHAR_MAX, (float)ny / CHAR_MAX, (float)nz / CHAR_MAX}; }

	void vec3_to_normal(vmath::vec3 in)
		{ nx = (int8_t)(in.x * CHAR_MAX); ny = (int8_t)(in.y * CHAR_MAX); nz = (int8_t)(in.z * CHAR_MAX); }

	vmath::vec3 tangent_to_vec3()
		{ return {(float)tx / CHAR_MAX, (float)ty / CHAR_MAX, (float)tz / CHAR_MAX}; }

	void vec3_to_tangent(vmath::vec3 in)
		{ tx = (int8_t)(in.x * CHAR_MAX); ty = (int8_t)(in.y * CHAR_MAX); tz = (int8_t)(in.z * CHAR_MAX); }


}; // 12 bytes; can possibly reduce to 8 by using qtangents.

struct VertexSkin
{
	uint8_t bone[4];
	uint8_t weight[4];

}; // 8 bytes; could reduce to 4 by limiting to 2 bones per vertex, but do we really want to?

struct VertexSkinLarge
{
	uint16_t bone[4];
	uint8_t weight[4];

}; // 12 bytes

struct Vertex2D
{
	float x, y, s, t;
};

inline uint32_t calc_bytes_per_vertex(uint32_t format)
{
	uint32_t result = 0;

	if (format & VF_POSITION)
		result += sizeof(VertexPosition);
	if (format & VF_SURFACE)
		result += sizeof(VertexSurface);
	if (format & VF_SKIN)
		result += sizeof(VertexSkin);
	if (format & VF_SKIN_LARGE)
		result += sizeof(VertexSkinLarge);
	if (format & VF_2D)
		result += sizeof(Vertex2D);

	return result;
}

size_t InitVertex(VertexFormatFlags format, uint32_t num_vertices, size_t offset = 0);

#endif // HVH_WC_GRAPHICS_VERTEX_H