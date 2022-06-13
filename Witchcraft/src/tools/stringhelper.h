#ifndef HVH_WC_TOOLS_STRINGHELPER_H
#define HVH_WC_TOOLS_STRINGHELPER_H

#include <string>
#include <sstream>
#include <vector>
#include <codecvt>
#include <locale>

inline std::wstring utf8_to_utf16(const std::string& in)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t, 1114111UL, std::little_endian>, wchar_t> converter;
	return converter.from_bytes(in);
}

inline std::string utf16_to_utf8(const std::wstring& in)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.to_bytes(in);
}

inline std::basic_string<uint32_t> utf8_to_utf32(const std::string& in)
{
	std::wstring_convert<std::codecvt_utf8<uint32_t, 1114111UL, std::little_endian>, uint32_t> converter;
	return converter.from_bytes(in);
}

inline std::string utf32_to_utf8(const std::basic_string<uint32_t>& in)
{
	std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> converter;
	return converter.to_bytes(in);
}

inline std::vector<std::string> splitstr(const std::string& str, char delim = ' ')
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delim))
		{ result.push_back(item); }

	return result;
}

inline std::vector<std::string> splitstr_nondestructive(const std::string& str, char delim = ' ')
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delim))
	{
		item += delim;
		result.push_back(item);
	}

	if (str.size() > 0 && str.back() != delim)
		{ result.back().pop_back(); }

	return result;
}

inline std::vector<std::string> split_by_tag(std::string str, char tagstart = '<', char tagend = '>')
{
	std::vector<std::string> result;
	int startpos = 0;
	int endpos = 0;
	// Look for the tag end character (>)
	for (endpos = 0; endpos < (int)str.size(); ++endpos)
	{
		if (str[endpos] == tagend)
		{
			// Once we find it, we go backwards looking for the matching tag start (<)
			for (startpos = endpos; startpos >= 0; --startpos)
			{
				if (startpos != 0)
					{ result.push_back(str.substr(0, startpos)); }

				endpos++;
				result.push_back(str.substr(startpos, endpos - startpos));
				str = str.substr(endpos);
				endpos = -1;
				break;
			}
		}
	}

	if (str.size() > 0)
		result.push_back(str);

	return result;
}

inline void lowercase(char* str)
{
	if (str == nullptr) return;
	for (size_t i = 0; str[i] != '\0'; ++i)
		{ str[i] = tolower(str[i]); }
}

inline void lowercase(std::string& str)
{
	for (char& c : str)
		{ c = tolower(c); }
}

inline void lowercase_proper(std::string& str)
{
	std::locale loc;
	std::basic_string<uint32_t> wstr = utf8_to_utf32(str);
	for (uint32_t& c : wstr)
		{ c = std::tolower(c, loc); }
	str = utf32_to_utf8(wstr);
}

inline void strip_backslashes(char* str)
{
	if (str == nullptr) return;
	for (size_t i = 0; str[i] != '\0'; ++i)
		{ if (str[i] == '\\') str[i] = '/'; }
}

inline void strip_backslashes(std::string& str)
{
	for (char& c : str)
		{ if (c == '\\') c = '/'; }
}


#endif // HVH_WC_TOOLS_STRINGHELPER_H