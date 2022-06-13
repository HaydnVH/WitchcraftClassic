#include "module.h"

#include <set>
#include <fstream>
using namespace std;

#include "sys/printlog.h"

#include <filesystem>
namespace fs = std::filesystem;
#include "file.h"

#include <pugixml.hpp>
#include "tools/xmlhelper.h"
using namespace pugi;

constexpr const char* MOD_INFO_FILENAME = "module.xml";

static const set<string> reservedFilenames = { "module.xml", "readme.txt", "splash.png", "config.xml", "load_order.xml" };

void Module::close()
{
	file_list.clear();
	known_conflicts.clear();
	dependancies.clear();
	archive.close();
	enabled = false;
	found = false;
	loaded = false;
}

void Module::open()
{
	fs::path mypath = fs::u8path(path);

	if (!fs::exists(mypath))
		return;

	if (!fs::is_directory(mypath))
	{
		archive.open(path.c_str());
	}
}

bool Module::load_mod_info()
{
	fs::path mypath = fs::u8path(path);

	if (!fs::exists(mypath))
		return false;

	InFile modinfo;

	// Check to see whether or not we're a directory, and try to open up module.xml
	if (fs::is_directory(mypath))
	{
		modinfo.open(mypath / MOD_INFO_FILENAME);
		if (modinfo.is_open() == false)
		{
			return false;
		}
	}
	else
	{
		archive.open(mypath.u8string().c_str());
		size_t size = 0;
		char* ptr = archive.extract_data(MOD_INFO_FILENAME, NULL, &size, NULL, true);
		if (!ptr)
		{
			return false;
		}

		modinfo.open(ptr, size);
	}

	string contents = modinfo.contents();

	xml_document doc;
	xml_node root;
	xml_parse_result result = doc.load_buffer_inplace(&contents[0], contents.size());
	if (!result || !(root = doc.first_child()))
	{
		plog::error("In Module::loadModInfo():\n");
		plog::errmore("Error parsing '%s/%s'\n", name.c_str(), MOD_INFO_FILENAME);
		plog::errmore("Description: %s", result.description());
		plog::errmore("Offset: %s", result.offset);
		plog::errmore("(error at [...%s]\n", &contents[result.offset]);
		return false;
	}

	const char* verstring = readXML(root, "version", "");
	sscanf(verstring, "%hhi.%hhi.%hi", &majorver, &minorver, &patchver);
	author = readXML(root, "author", "UNDEFINED_AUTHOR");
	description = readXML(root, "description", "UNDEFINED_DESCRIPTION");
	category = readXML(root, "category", "UNDEFINED_CATEGORY");
	priority = readXML(root, "load_priority", -1.0f);

	xml_node node;
	if (node = root.child("dependancies"))
	{
		for (xml_node child = node.child("module"); child; child = child.next_sibling("module"))
		{
			xml_node name = child.child("name");
			if (!name || !name.value())
				continue;

			ModReference modref;
			modref.name = name.value();
			modref.note = readXML(child, "note", "");

			const char* verstring = readXML(root, "version", "");
			sscanf(verstring, "%hhi.%hhi.%hi", &modref.majorver, &modref.minorver, &modref.patchver);

			dependancies.push_back(modref);
		}
	}
	if (node = root.child("conflicts"))
	{
		for (xml_node child = node.child("module"); child; child = child.next_sibling("module"))
		{
			xml_node name = child.child("name");
			if (!name || !name.value())
				continue;

			ModReference modref;
			modref.name = name.value();
			modref.note = readXML(child, "note", "");

			const char* verstring = readXML(root, "version", "");
			sscanf(verstring, "%hhi.%hhi.%hi", &modref.majorver, &modref.minorver, &modref.patchver);

			known_conflicts.push_back(modref);
		}
	}

	archive.close();
	return true;
}

void search_module_recursive(vector<FixedFilePath>& list, Module* module, const fs::path& parent, fs::path dir)
{
	for (fs::directory_iterator it(parent / dir); it != fs::directory_iterator(); ++it)
	{
		// Path doesn't exist.  This case should never happen, but hey, just in case!
		if (fs::exists(it->path()) == false)
			continue;

		// Path is a directory,
		if (fs::is_directory(it->path()))
		{
			// So we need to go deeper.
			search_module_recursive(list, module, parent, dir / it->path().filename());
			continue;
		}

		// Path is a file
		string filepath = (dir / it->path().filename()).u8string();

		// Make sure it's not too long.
		if (filepath.size() > ARCHIVE_FILEPATH_MAX_STRLEN)
		{
			plog::error("In searchModuleRecursive():\n");
			plog::errmore("File path '%s' is too long.  Must be no more than %i bytes long (in UTF-8).\n", filepath.c_str(),  ARCHIVE_FILEPATH_MAX_STRLEN);
			continue;
		}

		// First we check to make sure it isn't a reserved filename.
		if (reservedFilenames.count(filepath))
			continue;

		FixedFilePath entry;
		strcpy(entry.path, filepath.c_str());

		// Fix backslashes!
		strip_backslashes(entry.path);

		// Then we insert it into the list.
		auto pos = lower_bound(list.begin(), list.end(), filepath, [](const FixedFilePath& lhs, const string& rhs) { return strcmp(lhs.path, rhs.c_str()) > 0; });
		if (pos != list.end() && (strcmp(pos->path, filepath.c_str()) == 0))
		{
			plog::error("In searchModuleRecursive():\n");
			plog::errmore(" File path '%s' somehow appeared twice?\n", filepath.c_str());
			continue;
		}

		list.insert(pos, entry);
	}
}

bool Module::load_file_list()
{
	unload_file_list();

	if (archive.is_open())
	{
		// An archive's dictionary is already formatted in the style we're looking for,
		// so we just copy it straight-in.
		file_list.resize(archive.num_files());
		file_list = archive.file_list();
	}
	else
	{
		search_module_recursive(file_list, this, path, "");
	}

	return true;
}

void Module::unload_file_list()
{
	file_list.clear();
}

bool Module::is_archive()
{
	if (fs::is_directory(fs::u8path(path)))
		return false;
	else
		return true;
}