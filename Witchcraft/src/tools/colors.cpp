#include "colors.h"

#include <map>
#include <string>
#include <cstring>
using namespace std;
using namespace vmath;

namespace {

map<string, vec4> known_colors =
{
	{"red", {1, 0, 0, 1}},
	{"green", {0, 1, 0, 1}},
	{"blue", {0, 0, 1, 1}},
	{"cyan", {0, 1, 1, 1}},
	{"magenta", {1, 0, 1, 1}},
	{"yellow", {1, 1, 0, 1}},
	{"white", {1, 1, 1, 1}},
	{"black", {0, 0, 0, 1}},
	{"gray", {0.5f, 0.5f, 0.5f, 1}},
	{"info", {0.06f, 0.94f, 0.12f, 1}},
	{"warning", {0.94f, 0.87f, 0.06f, 1}},
	{"error", {0.87f, 0.12f, 0, 1}}
};

} // namespace <anon>

namespace colors {

bool is_valid(const char* name)
{
	vec4 dummy;
	return (get(name, dummy));
}

bool get(const char* name, vec3& color)
{
	if (known_colors.count(name) == 0)
		{ return parse(name, color); }
	else
		{ color = known_colors[name].rgb; }

	return true;
}

bool get(const char* name, vec4& color)
{
	if (known_colors.count(name) == 0)
		{ return parse(name, color); }
	else
		{ color = known_colors[name]; }

	return true;
}

bool parse(const char* str, vec3& color)
{
	size_t strsize = strlen(str);
	unsigned char r, g, b;
	if (strsize == 6 && sscanf_s(str, "%2hhx%2hhx%2hhx", &r, &g, &b) == 3)
	{
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		return true;
	}
	else if (strsize == 3 && sscanf_s(str, "%1hhx%1hhx%1hhx", &r, &g, &b) == 3)
	{
		color.r = (float)r / 15.0f;
		color.g = (float)g / 15.0f;
		color.b = (float)b / 15.0f;
		return true;
	}
	else
		return false;
}

bool parse(const char* str, vec4& color)
{
	size_t strsize = strlen(str);
	unsigned char r, g, b, a;
	if (strsize == 8 && sscanf_s(str, "%2hhx%2hhx%2hhx%2hhx", &r, &g, &b, &a) == 4)
	{
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		color.a = (float)a / 255.0f;
		return true;
	}
	else if (strsize == 4 && sscanf_s(str, "%1hhx%1hhx%1hhx%1hhx", &r, &g, &b, &a) == 4)
	{
		color.r = (float)r / 15.0f;
		color.g = (float)g / 15.0f;
		color.b = (float)b / 15.0f;
		color.a = (float)a / 15.0f;
		return true;
	}
	else if (strsize == 6 && sscanf_s(str, "%2hhx%2hhx%2hhx", &r, &g, &b) == 3)
	{
		color.r = (float)r / 255.0f;
		color.g = (float)g / 255.0f;
		color.b = (float)b / 255.0f;
		color.a = 1.0f;
		return true;
	}
	else if (strsize == 3 && sscanf_s(str, "%1hhx%1hhx%1hhx", &r, &g, &b) == 3)
	{
		color.r = (float)r / 15.0f;
		color.g = (float)g / 15.0f;
		color.b = (float)b / 15.0f;
		color.a = 1.0f;
		return true;
	}
	else
		return false;
}

} // namespace colors