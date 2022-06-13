#include "font.h"

#include "filesystem/file_manager.h"
#include "tools/xmlhelper.h"
#include "sys/printlog.h"

using namespace std;
using namespace pugi;

constexpr const char* FONTS_FOLDER = "fonts/";

constexpr const int ASCII_RANGE_START = 32; // 0 through 31 are non-human-readable control codes.
constexpr const int ASCII_RANGE_END = 127; // 127 is DELETE, also non-human-readable.
constexpr const int ASCII_RANGE_SIZE = ASCII_RANGE_END - ASCII_RANGE_START;
// Anything greater than 127 is Unicode.

constexpr const unsigned int UNCERTAINTY_GLYPH = 0x2bd1;

Font::Font(const char* name)
:	glyphs({}),
	size(1),
	newline(1),
	scalable(false)
{
	string xmlpath = string(FONTS_FOLDER) + string(name) + "/" + string(name) + string(".fnt");
	InFile fontfile = filemanager::LoadSingleFile(xmlpath.c_str(), ios::in);
	if (fontfile.is_open() == false)
		{ plog::error("Failed to open font file '%s'.\n", xmlpath.c_str()); return; }

	// Load in the file's contents.
	stringstream ss;
	ss << fontfile.rdbuf();
	string file_contents = ss.str();

	// Parse the file.
	xml_document doc;
	xml_parse_result parse_result = doc.load_buffer_inplace((void*)file_contents.data(), file_contents.size());
	if (!parse_result)
		{ plog::error("Error parsing file '%s'.\n", xmlpath.c_str()); return; }

	xml_node root = doc.child("font");
	if (!root)
		{ plog::error("Could not find 'font' root node.\n"); return; }

	xml_node infonode = root.child("info");
	if (!infonode)
		{ plog::error("Could not find 'info' node.\n"); }
	size = infonode.attribute("size").as_int(1);

	xml_node commonnode = root.child("common");
	if (!commonnode)
		{ plog::error("Could not find 'common' node.\n"); }
	newline = commonnode.attribute("lineHeight").as_int(1);

	xml_node pagesnode = root.child("pages");
	if (!pagesnode)
		{ plog::error("Could not find 'pages' node.\n"); }
	xml_node page0node = pagesnode.child("page");
	string imagepath = string(FONTS_FOLDER) + string(name) + "/" + string(page0node.attribute("file").as_string());

	texture.LoadImage(imagepath.c_str());
	texture.setWrapping(TEXWRAP_CLAMP, TEXWRAP_CLAMP);

	xml_node charsnode = root.child("chars");
	if (!charsnode)
		{ plog::error("Could not find 'chars' node.\n"); return; }

	xml_node distancefield_node = root.child("distanceField");
	if (distancefield_node)
		{ scalable = true; }

	// Reserve space for all of the human-readable ASCII characters.
	// Every font must supply this set.
	int numglyphs = charsnode.attribute("count").as_int(0);
	if (numglyphs < ASCII_RANGE_SIZE)
		{ plog::error("Font does not contain enough glyphs to fill the ASCII range.\n"); }

	glyphs.reserve(numglyphs);
	glyphs.resize(ASCII_RANGE_SIZE);

	for (xml_node glyphnode = charsnode.child("char"); glyphnode; glyphnode = glyphnode.next_sibling("char"))
	{
		GlyphInfo glyph = {};
		glyph.codepoint = glyphnode.attribute("id").as_uint();
		glyph.u = glyphnode.attribute("x").as_uint();
		glyph.v = glyphnode.attribute("y").as_uint();
		glyph.width = glyphnode.attribute("width").as_uint();
		glyph.height = glyphnode.attribute("height").as_uint();
		glyph.xoffset = glyphnode.attribute("xoffset").as_int();
		glyph.yoffset = glyphnode.attribute("yoffset").as_int();
		glyph.xadv = glyphnode.attribute("xadvance").as_int();
		
		// If this glyph is ASCII, we insert it directly into the list easily.
		if (glyph.codepoint < ASCII_RANGE_END)
			{ glyphs[glyph.codepoint - ASCII_RANGE_START] = glyph; }
		else
		{
			// If it's not, we do a binary search to determine where to insert it.
			auto it = lower_bound(glyphs.begin() + ASCII_RANGE_SIZE, glyphs.end(), glyph.codepoint);
			glyphs.insert(it, glyph);
		}
	
	} // for glyphnodes

	// Make sure that all of ASCII is present.
	for (int i = 0; i < ASCII_RANGE_SIZE; ++i)
	{
		if (glyphs[i].codepoint != (i + ASCII_RANGE_START))
			{ plog::error("Invalid or missing glyph data for ASCII '%c'.\n", (char)(i + ASCII_RANGE_START)); }
	}

	// TODO: Kerning pairs?
}

Font::~Font()
{}

GlyphInfo Font::getGlyphInfo(uint32_t codepoint, uint32_t style)
{
	// TODO: use style

	if (codepoint < ASCII_RANGE_END && codepoint >= ASCII_RANGE_START)
		return glyphs[codepoint - ASCII_RANGE_START];
	else
	{
		GlyphInfo result = {};
		auto it = lower_bound(glyphs.begin() + ASCII_RANGE_SIZE, glyphs.end(), codepoint);
		if (it == glyphs.end())
		{
			if (codepoint == UNCERTAINTY_GLYPH)
				return getGlyphInfo('?', style);
			else
				return getGlyphInfo(UNCERTAINTY_GLYPH, style);
		}
		else
			return *it;
	}
}