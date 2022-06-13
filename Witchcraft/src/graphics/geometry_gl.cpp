#ifdef RENDERER_OPENGL

#include "geometry.h"
#include <GL/glew.h>
#include <GL/GL.h>

#include <map>

#include "sys/printlog.h"

namespace
{
	uint32_t boundArray = 0;

	inline void bind(uint32_t vao)
	{
		if (boundArray == vao) return;
		glBindVertexArray(vao);
		boundArray = vao;
	}

	inline uint32_t index_size(GLenum iformat)
	{
		switch (iformat)
		{
		case GL_UNSIGNED_BYTE:	return 1;
		case GL_UNSIGNED_SHORT:	return 2;
		case GL_UNSIGNED_INT:	return 4;
		default:				return 0;
		}
	}

	std::map<std::string, GeomEnum> str_to_enum =
	{
		{ "invalid", GEOMETRY_PRIMITIVE_INVALID },
		{ "points", GEOMETRY_PRIMITIVE_POINTS },
		{ "lines", GEOMETRY_PRIMITIVE_LINES },
		{ "line loop", GEOMETRY_PRIMITIVE_LINE_LOOP },
		{ "line strip", GEOMETRY_PRIMITIVE_LINE_STRIP },
		{ "triangles", GEOMETRY_PRIMITIVE_TRIANGLES },
		{ "triangle strip", GEOMETRY_PRIMITIVE_TRIANGLE_STRIP },
		{ "triangle fan", GEOMETRY_PRIMITIVE_TRIANGLE_FAN },
		{ "quads", GEOMETRY_PRIMITIVE_QUADS },
		{ "quad strip", GEOMETRY_PRIMITIVE_QUAD_STRIP }
	};
}

GeomEnum geom_enum_from_str(const char* str)
{
	if (str_to_enum.count(str) == 0)
		return GEOMETRY_PRIMITIVE_INVALID;
	else
		return str_to_enum[str];
}

void Geometry::Clean()
{
	if (boundArray == vao_)
		bind(0);

	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
		vbo_ = 0;
	}

	if (ibo_)
	{
		glDeleteBuffers(1, &ibo_);
		ibo_ = 0;
	}

	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}

	ready_ = false;
}

void Geometry::Load(GeomEnum primitive, void* vertices, VertexFormatFlags vert_format, uint32_t vert_count, void* indices, int isize, uint32_t index_count)
{
	// No offline vertex loading yet, so spit out an error if we weren't given any vertices.
	if (vertices == NULL || calc_bytes_per_vertex(vert_format) == 0 || vert_count == 0)
	{
		plog::error("In Geometry::load_gl():\n");
		plog::errmore("Invalid vertices provided!\n");
		return;
	}

	// Clean up any existing data
	Clean();

	// Set up the Vertex Array Object
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	boundArray = vao_;

	// Create the Vertex Buffer Object
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, calc_bytes_per_vertex(vert_format) * vert_count, vertices, GL_STATIC_DRAW);

	// Save some things for later
	primtype_ = GL_POINTS + primitive;
	vformat_ = vert_format;
	vcount_ = vert_count;

	// If we've been given index data, handle it //
	if (indices != NULL && index_count > 0)
	{
		glGenBuffers(1, &ibo_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize * index_count, indices, GL_STATIC_DRAW);

		switch (isize)
		{
		case 1: iformat_ = GL_UNSIGNED_BYTE; break;
		case 2: iformat_ = GL_UNSIGNED_SHORT; break;
		case 4: iformat_ = GL_UNSIGNED_INT; break;
		default:
		{
			plog::error("In Geometry::load_gl():\n");
			plog::error("Invalid index type!\n");
			Clean();
			return;
		}
		}

		icount_ = index_count;
	}

	// Initialize array pointers according to the vertex type
	InitVertex(vert_format, vert_count);

	// Unbind the vertex array
	bind(0);

	// And we're done here!
	ready_ = true;
}

void Geometry::Draw(uint32_t start, int32_t count)
{
	if (ready_ == false)
		return;

	if (count < 0)
	{
		if (ibo_)
			count = icount_;
		else
			count = vcount_;
	}

	// Bind our vertex array //
	bind(vao_);

	// Draw it's contents //
	if (ibo_ == 0)
		glDrawArrays(primtype_, start, count);
	else
		glDrawElements(primtype_, count, iformat_, (GLvoid*)(size_t)(index_size(iformat_)*start));

//	bind(0);
}
/*
void Geometry::DrawElements(uint32_t start, uint32_t count)
{
	if (ready_ == false)
		return;

	bind(vao_);

	glDrawElements(primtype_, count, iformat_, (GLvoid*)(size_t)(index_size(iformat_)*start));
}
*/
void Geometry::DrawFullscreenQuad()
{
	static Geometry quad;

	if (!quad.is_ready())
	{
		VertexPosition vertices[] = 
		{
			{-1, 1, 0},
			{-1, -1, 0},
			{1, 1, 0},
			{1, -1, 0}
		};
		quad.Load(GEOMETRY_PRIMITIVE_TRIANGLE_STRIP, vertices, VF_POSITION, 4);
	}

	quad.Draw();
}

void Geometry::DrawWireCube()
{
	static Geometry cube;

	if (!cube.is_ready())
	{
		VertexPosition vertices[] = 
		{
			{-1, -1, -1},
			{-1, -1, 1},
			{-1, 1, -1},
			{-1, 1, 1},
			{1, -1, -1},
			{1, -1, 1},
			{1, 1, -1},
			{1, 1, 1}
		};

		unsigned char indices[] =
		{
			0, 1,
			1, 3,
			2, 3,
			2, 0,
			4, 5,
			5, 7,
			6, 7,
			6, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		cube.Load(GEOMETRY_PRIMITIVE_LINES, vertices, VF_POSITION, 8, indices, sizeof(unsigned char), 24);
	}

	cube.Draw();
}

#endif // RENDERER_OPENGL