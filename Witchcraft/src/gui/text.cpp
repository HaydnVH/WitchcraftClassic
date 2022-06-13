#include "text.h"

#include "tools/stringhelper.h"

using namespace std;
using namespace vmath;

void Text::Draw(const UniformBufferCamera& camera)
{
	if (saved_aspect_ratio != camera.aspect_ratio)
	{
		saved_aspect_ratio = camera.aspect_ratio;
		needs_rebuild = true;
	}

	if (needs_rebuild)
	{
		vector<Vertex2D> text_vertices;
		float xcursor = 0.0f, ycursor = 0.0f;
		MakeGeometry(text_vertices, text, camera.aspect_ratio, size, xcursor, ycursor);
		geom.Load(GEOMETRY_PRIMITIVE_TRIANGLES, text_vertices.data(), VF_2D, (uint32_t)text_vertices.size());
		needs_rebuild = false;
	}

	geom.Draw();
}

void Text::MakeGeometry(std::vector<Vertex2D>& text_vertices, string str, float aspect, float scale, float& xcursor, float& ycursor)
{
	float vertical_text_factor = 1.0f;
	float horizontal_text_factor = 1.0f;

	if (font.isScalable())
	{
		// This text factors transform font coordinates from [0, 1080] to [0, 1].
		// We apply the scale factor here, as well.
		vertical_text_factor = (NORMALIZED_VERTICAL_TEXT_FACTOR / (float)font.getSize()) * scale;
		// By taking the aspect ratio into account we can scale horizontal coordsinates from [0, 1920] to [0, 1].
		horizontal_text_factor = vertical_text_factor / aspect;
	}

	// For each codepoint...
	std::basic_string<uint32_t> utf32_str = utf8_to_utf32(str);
	for (uint32_t codepoint : utf32_str)
	{
		GlyphInfo glyph = font.getGlyphInfo(codepoint);
		if (glyph.codepoint != codepoint)
			continue;

		if (glyph.width > 0 && glyph.height > 0)
		{
			Vertex2D vertex = {};
			// Generate a quad from the glyph info, and push it onto the array.

			vertex.x = xcursor + (glyph.xoffset * horizontal_text_factor);
			vertex.y = ycursor + (glyph.yoffset * vertical_text_factor);
			vertex.s = (float)glyph.u; vertex.t = (float)glyph.v;
			text_vertices.push_back(vertex);

			vertex.x = xcursor + (glyph.xoffset * horizontal_text_factor);
			vertex.y = ycursor + ((glyph.yoffset + glyph.height) * vertical_text_factor);
			vertex.s = (float)glyph.u; vertex.t = (float)(glyph.v + glyph.height);
			text_vertices.push_back(vertex);

			vertex.x = xcursor + ((glyph.xoffset + glyph.width) * horizontal_text_factor);
			vertex.y = ycursor + (glyph.yoffset * vertical_text_factor);
			vertex.s = (float)(glyph.u + glyph.width); vertex.t = (float)glyph.v;
			text_vertices.push_back(vertex);

			vertex.x = xcursor + ((glyph.xoffset + glyph.width) * horizontal_text_factor);
			vertex.y = ycursor + (glyph.yoffset * vertical_text_factor);
			vertex.s = (float)(glyph.u + glyph.width); vertex.t = (float)glyph.v;
			text_vertices.push_back(vertex);

			vertex.x = xcursor + (glyph.xoffset * horizontal_text_factor);
			vertex.y = ycursor + ((glyph.yoffset + glyph.height) * vertical_text_factor);
			vertex.s = (float)glyph.u; vertex.t = (float)(glyph.v + glyph.height);
			text_vertices.push_back(vertex);

			vertex.x = xcursor + ((glyph.xoffset + glyph.width) * horizontal_text_factor);
			vertex.y = ycursor + ((glyph.yoffset + glyph.height) * vertical_text_factor);
			vertex.s = (float)(glyph.u + glyph.width); vertex.t = (float)(glyph.v + glyph.height);
			text_vertices.push_back(vertex);
		}

		float advance = glyph.xadv * horizontal_text_factor;
		xcursor += advance;

	} // For each codepoint

	needs_rebuild = false;
}