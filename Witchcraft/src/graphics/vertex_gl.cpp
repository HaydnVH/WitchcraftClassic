#ifdef RENDERER_OPENGL
#include "vertex.h"
#include <GL/glew.h>
#include <GL/GL.h>

#include "sys/printlog.h"

size_t InitVertex(VertexFormatFlags format, uint32_t num_vertices, size_t offset)
{
	if ((format & (VF_POSITION | VF_SURFACE | VF_SKIN | VF_SKIN_LARGE)) && (format & VF_2D))
	{
		plog::error("2D vertices should not have 3D information.\n");
		return offset;
	}

	if ((format & VF_SKIN) && (format & VF_SKIN_LARGE))
	{
		plog::error("Normal skinning information and 'large' skinning information are mutually exclusive.\n");
		return offset;
	}

	if (format & VF_POSITION)
	{
		// Position uses index 0 and is a 'float3'.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
		offset += sizeof(VertexPosition) * num_vertices;
	}

	if (format & VF_SURFACE)
	{
		// Texture coordinates use index 1 and is a 'half2'
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_HALF_FLOAT, GL_FALSE, sizeof(VertexSurface), (void*)(offset + offsetof(VertexSurface, s)));
		// Normals use index 2 and is a 'byte4'
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_BYTE, GL_TRUE, sizeof(VertexSurface), (void*)(offset + offsetof(VertexSurface, nx)));
		// Tangents use index 3 and is a 'byte4'
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_BYTE, GL_TRUE, sizeof(VertexSurface), (void*)(offset + offsetof(VertexSurface, tx)));
		offset += sizeof(VertexSurface) * num_vertices;
	}

	if (format & VF_SKIN)
	{
		// Bone indices use index 4 and is a 'ubyte4'.
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_UNSIGNED_BYTE, sizeof(VertexSkin), (void*)(offset + offsetof(VertexSkin, bone)));
		// Bone weights use index 5 and is a 'ubyte4'.
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexSkin), (void*)(offset + offsetof(VertexSkin, weight)));
		offset += sizeof(VertexSkin) * num_vertices;
	}

	if (format & VF_SKIN_LARGE)
	{
		// Bone indices use index 4 and is a 'ushort4'.
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_UNSIGNED_SHORT, sizeof(VertexSkinLarge), (void*)(offset + offsetof(VertexSkinLarge, bone)));
		// Bone weights use index 5 and is a 'ubyte4'.
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexSkinLarge), (void*)(offset + offsetof(VertexSkinLarge, weight)));
		offset += sizeof(VertexSkinLarge) * num_vertices;
	}

	if (format & VF_2D)
	{
		// 2D vertex data is packed such that the position is in xy and the texture coordinate is in zw.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)(offset + offsetof(Vertex2D, x)));
	}

	return offset;
}

#endif