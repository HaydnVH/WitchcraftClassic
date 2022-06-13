#ifndef HVH_WC_GUI_TEXTWRAP_H
#define HVH_WC_GUI_TEXTWRAP_H

#include "text.h"

float WordWidth(const std::string& str, Font& font, float aspect, float scale = 1.0f);

class TextWrap : public Text
{
public:
	TextWrap(Font& font)
	:	Text(font),
		dimensions(0, 0)
	{}

	void setWidth(float width)
		{ dimensions.x = width; needs_rebuild = true; }

	void setLineHeight(int numlines)
		{ dimensions.y = font.getSize() * 1.5f * (float)numlines; needs_rebuild = true; }
	
	void setDimensions(vmath::vec2 newdimensions)
		{ dimensions = newdimensions; needs_rebuild = true; }

	virtual void Draw(const UniformBufferCamera& camera)
	{
		if (saved_aspect_ratio != camera.aspect_ratio)
		{
			saved_aspect_ratio = camera.aspect_ratio;
			needs_rebuild = true;
		}

		if (needs_rebuild)
			{ BuildLines(camera.aspect_ratio); }

		for (int i = 0; i < (int)lines.size(); ++i)
			{ DrawLine(camera, i); }
	}

	void DrawLine(const UniformBufferCamera& camera, int line, int num_glyphs = -1);

	int getNumGlyphsOnLine(int line)
	{
		if (line < 0 || line >= lines.size())
			return 0;
		else
			return lines[line].vcount / 6;
	}

	int getNumLines()
		{ return (int)lines.size(); }

	void BuildLines(float aspect);

private:

	vmath::vec2 dimensions;

	struct PerLine
		{ uint32_t vstart, vcount; };

	std::vector<PerLine> lines;
};

#endif // HVH_WC_GUI_TEXTWRAP_H