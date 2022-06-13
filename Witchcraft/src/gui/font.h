#ifndef HVH_WC_GUI_FONT_H
#define HVH_WC_GUI_FONT_H

#include <vector>
#include <string>
#include <map>
#include <cstdint>

#include "graphics/texture.h"

struct GlyphInfo
{
	// The highest codepoint in Unicode 6.0 is 0x10ffff, which can be fit in 3 bytes.
	// Using this, we can get the sizeof the GlyphInfo struct down to a nice even 12 bytes.
	uint32_t codepoint : 24;
	int32_t xadv : 8;

	uint16_t u, v;
	uint8_t width, height;
	int8_t xoffset, yoffset;

	inline bool operator < (const GlyphInfo& rhs)
		{ return codepoint < rhs.codepoint; }

	inline bool operator < (uint32_t rhs)
		{ return codepoint < rhs; }
};

class Font
{
public:
	Font(const char* name);
	~Font();

	GlyphInfo getGlyphInfo(uint32_t codepoint, uint32_t style = 0);

	short getSize()
		{ return size; }

	short getNewLine()
		{ return newline; }

	bool isScalable()
		{ return scalable; }

	void useTexture(uint32_t index)
		{ texture.Use2D(index); }

private:
	std::vector<GlyphInfo> glyphs;
	short size;
	short newline;
	bool scalable;

	Texture texture;
};

#endif // HVH_WC_GUI_FONT_H