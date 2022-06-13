#include "text_wrap.h"

#include "tools/stringhelper.h"

using namespace std;

float WordWidth(const string& str, Font& font, float aspect, float scale)
{
	float horizontal_text_factor = (NORMALIZED_VERTICAL_TEXT_FACTOR / font.getSize()) * scale * (1.0f/aspect);

	float result = 0.0f;

	std::basic_string<uint32_t> utf32_str = utf8_to_utf32(str);
	for (uint32_t codepoint : utf32_str)
	{
		GlyphInfo glyph = font.getGlyphInfo(codepoint);
		result += (glyph.xadv * horizontal_text_factor);
	}

	return result;
}

void TextWrap::DrawLine(const UniformBufferCamera& camera, int line, int num_glyphs)
{
	if (saved_aspect_ratio != camera.aspect_ratio)
	{
		saved_aspect_ratio = camera.aspect_ratio;
		needs_rebuild = true;
	}

	if (needs_rebuild)
		{ BuildLines(camera.aspect_ratio); }

	if (line < 0 || line >= lines.size())
		return;

	uint32_t count = 0;
	if (num_glyphs < 0 || (num_glyphs * 6) > (int)lines[line].vcount)
		count = lines[line].vcount;
	else
		count = num_glyphs * 6;

	geom.Draw(lines[line].vstart, count);
}


void TextWrap::BuildLines(float aspect)
{
	// Split up our block of text into lines.
	vector<string> mylines = splitstr(text, '\n');

	// Split up each line into words (keep the ' ', though).
	vector<vector<string>> words;
	for (size_t i = 0; i < mylines.size(); ++i)
		{ words.push_back(splitstr_nondestructive(mylines[i], ' ')); }

	// Now we have to measure the width of each word and compare the total width against the width of the text box.
	// If a word is too wide, we push it onto the next line (creating a new line if neccesary).
	// If a word is too wide and it's the first word on the line, we can split it up to fit on multiple lines.

	float curwidth = 0.0f;

	for (size_t i = 0; i < words.size(); ++i)
	{
		curwidth = 0.0f;
		for (size_t j = 0; j < words[i].size(); ++j)
		{
			// Get the width of this word.
			curwidth += WordWidth(words[i][j], font, aspect, size);

			// If we've gone too far...
			if (curwidth > dimensions.x)
			{
				// If this is the first word on the line, AND it's too wide, we split it up.
				if (j == 0)
				{

				}
				// Otherwise, we move it to the next line.
				else
				{
					// If this is not the last line, we move the rest of this line to the beginning of the next line.
					if ((i+1) < words.size())
					{
						words[i + 1].insert(words[i + 1].begin(), words[i].begin() + j, words[i].end());
						words[i].erase(words[i].begin() + j, words[i].end());
					}
					// Otherwise, we create a new line.
					else
					{
						words.push_back({});
						words.back().insert(words.back().begin(), words[i].begin() + j, words[i].end());
						words[i].erase(words[i].begin() + j, words[i].end());
					}
				}
			}
		} // For each word in line.
	} // For each line.

	lines.clear();

	// Now that we have the words for each line, we can create the geometry.
	float xcursor = 0.0f, ycursor = 0.0f;
	vector<Vertex2D> text_vertices;
	for (auto& line : words)
	{
		PerLine perline = {};
		perline.vstart = (uint32_t)text_vertices.size();

		for (auto& word : line)
		{ MakeGeometry(text_vertices, word, aspect, size, xcursor, ycursor); }

		perline.vcount = (uint32_t)text_vertices.size() - perline.vstart;

		xcursor = 0.0f;
		ycursor += NORMALIZED_VERTICAL_TEXT_FACTOR * 1.5f;
		lines.push_back(perline);
	}

	geom.Load(GEOMETRY_PRIMITIVE_TRIANGLES, text_vertices.data(), VF_2D, (uint32_t)text_vertices.size());
	needs_rebuild = false;
}