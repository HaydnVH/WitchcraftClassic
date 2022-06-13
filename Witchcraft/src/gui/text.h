#ifndef HVH_WC_GUI_TEXT_H
#define HVH_WC_GUI_TEXT_H

#include <vector>
#include <string>

#include "font.h"

#include "graphics/camera.h"
#include "graphics/vertex.h"
#include "graphics/geometry.h"
#include "math/vmath.h"

constexpr const float DEFAULT_SCREEN_HEIGHT = 1080.0f;
constexpr const float DEFAULT_FONT_SIZE = 42.0f;
constexpr const float NORMALIZED_VERTICAL_TEXT_FACTOR = DEFAULT_FONT_SIZE / DEFAULT_SCREEN_HEIGHT;

enum TextShaderUniforms
{
	UNIFORM_TEXT_GLYPHS_TEXTURE,
	UNIFORM_TEXT_POSITION,
	UNIFORM_TEXT_COLOR,
	UNIFORM_TEXT_CUTOFFS,
	UNIFORM_TEXT_CLIPRECT,
	UNIFORM_TEXT_OUTLINE,
};

const std::vector<const char*> TEXT_SHADER_UNIFORM_NAMES =
{
	"tex_glyphs",
	"position",
	"color",
	"cutoffs",
	"cliprect",
	"outline",
};

class Text
{
public:
	Text(Font& font)
	:	font(font),
		saved_aspect_ratio(0.0f),
		position(0.0f, 0.0f),
		size(1.0f),
		needs_rebuild(false)
	{}

	void setPosition(vmath::vec2 newpos)
		{ position = newpos; }

	vmath::vec2 getPosition()
		{ return position; }

	void setString(std::string newstr)
		{ text = newstr; needs_rebuild = true; }

	void setSize(float newsize)
		{ size = newsize; needs_rebuild = true; }

	float getSize()
		{ return size; }

	virtual void Draw(const UniformBufferCamera& camera);

protected:

	void MakeGeometry(std::vector<Vertex2D>& text_vertices, std::string str, float aspect, float scale, float& xcursor, float& ycursor);

	Font& font;
	Geometry geom;

	float saved_aspect_ratio;
	vmath::vec2 position;
	float size;

	std::string text;
	bool needs_rebuild;
};

#endif // HVH_WC_GUI_TEXT_H