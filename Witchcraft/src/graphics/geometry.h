#ifndef HVH_WC_GRAPHICS_GEOMETRY_H
#define HVH_WC_GRAPHICS_GEOMETRY_H

#include "vertex.h"
#include <string.h>

enum GeomEnum
{
	GEOMETRY_PRIMITIVE_INVALID = -1,
	GEOMETRY_PRIMITIVE_POINTS = 0,
	GEOMETRY_PRIMITIVE_LINES,
	GEOMETRY_PRIMITIVE_LINE_LOOP,
	GEOMETRY_PRIMITIVE_LINE_STRIP,
	GEOMETRY_PRIMITIVE_TRIANGLES,
	GEOMETRY_PRIMITIVE_TRIANGLE_STRIP,
	GEOMETRY_PRIMITIVE_TRIANGLE_FAN,
	GEOMETRY_PRIMITIVE_QUADS,
	GEOMETRY_PRIMITIVE_QUAD_STRIP
//	GEOMETRY_PRIMITIVE_POLYGON
};

GeomEnum geom_enum_from_str(const char* str);

class Geometry
{
public:
	Geometry()
	{
		memset(this, 0, sizeof(Geometry));
	}
	Geometry(const Geometry& rhs) = delete;
	Geometry(Geometry&& rhs)
	{
		// Move constructor.
		memcpy(this, &rhs, sizeof(Geometry)); // copy rhs into this.
		memset(&rhs, 0, sizeof(Geometry)); // set rhs to 0s (so its destructor doesn't do anything dangerous).
	}
	inline Geometry& operator = (const Geometry& rhs) = delete;
	inline Geometry& operator = (Geometry&& rhs)
	{
		// Move-assignment operator, this is just copy-and-swap.
		Geometry temp; // temporary storage
		memcpy(&temp, this, sizeof(Geometry)); // copy this into temp
		memcpy(this, &rhs, sizeof(Geometry)); // copy rhs into this
		memcpy(&rhs, &temp, sizeof(Geometry)); // copy temp into rhs
		memset(&temp, 0, sizeof(Geometry)); // set temp to 0s
		return *this;
	}
	~Geometry() { Clean(); }

	void Clean();
	void Load(GeomEnum primitive, void* vertices, VertexFormatFlags vertFormat, uint32_t vertCount, void* indices = NULL, int indexSize = 0, uint32_t indexCount = 0);
	void Draw(uint32_t start = 0, int32_t count = -1);

	bool is_ready()
	{
		return ready_;
	}

	void static DrawWireCube();
	void static DrawFullscreenQuad();

private:
	bool ready_;

#ifdef RENDERER_OPENGL
	uint32_t vbo_, ibo_, vao_;
	uint32_t vcount_, icount_;
	uint32_t vformat_, iformat_, primtype_;
#elif RENDERER_VULKAN
#elif RENDERER_DIRECTX
#endif
};

#endif