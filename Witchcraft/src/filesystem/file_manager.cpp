#include "file_manager.h"

#include <vector>
#include <string>
#include <map>
#include <set>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

#include "sys/paths.h"
#include "sys/printlog.h"

#include <pugixml.hpp>
#include "tools/xmlhelper.h"
using namespace pugi;


constexpr const char* MODULE_EXTENSION = ".wc";
constexpr const char* MODULE_SAVE_EXT = ".sav";

constexpr const char* DATA_FOLDER = "modules/";
constexpr const char* SAVE_FOLDER = "save/";
constexpr const char* ENGINE_DATA_MODULE = "engine_data";

constexpr const char* LOAD_ORDER_FILENAME = "load_order.xml";

constexpr const char* ACTIVE_MODULE_PATH = "temp/active.sav";

struct FileEntry
{
	vector<Module*> locations;
};

namespace
{
	vector<Module> loaded_modules;
	set<string> loaded_module_names;

	Module active_module;

	map<string, FileEntry> all_files;

	void load_module(Module* module)
	{
		module->open();
		module->load_file_list();
		loaded_module_names.insert(module->get_name());

		for (auto& entry : module->get_file_list())
		{
			all_files[entry.path].locations.push_back(module);
		}
	}
} // namespace <anon>

namespace filemanager {

bool Init()
{
	if (isInitialized())
	{
		plog::fatal("Programmer Error: Filemanager cannot be initialized if it is already initialized!");
		return false;
	}

	// We need to load the 'engine data' module so we have something to work with.
	fs::path engine_data_path = fs::u8path(sys::getInstallPath()) / ENGINE_DATA_MODULE;

	// If it doesn't exist, we have an error.
	if (!fs::exists(engine_data_path))
	{
		plog::error("In sys::filesystem::startup():\n");
		plog::errmore("Failed to find '%s'.\n", ENGINE_DATA_MODULE);
		return false;
	}

	Module engine_data;
	engine_data.set_name(ENGINE_DATA_MODULE);
	engine_data.set_path(engine_data_path.u8string());

	if (!engine_data.load_mod_info())
	{
		plog::error("In sys::filesystem::startup():\n");
		plog::errmore("Failed to load '%s'.\n", ENGINE_DATA_MODULE);
		return false;
	}

	loaded_modules.push_back(engine_data);
	load_module(&loaded_modules.back());

	return true;

	// TODO: Load a "main menu" module?
}

void Shutdown()
{
	for (auto& it : loaded_modules)
		{ it.close(); }
	loaded_modules.clear();
	loaded_module_names.clear();
	all_files.clear();

	// This is just for debugging, it lets us see the insides of Active Module.
	active_module.get_archive()->unpack((fs::u8path(sys::getUserPath()) / "temp/extracted").u8string().c_str());

	active_module.close();

	error_code ec;
	fs::remove_all(fs::u8path(sys::getUserPath()) / ACTIVE_MODULE_PATH, ec);
}

bool isInitialized()
	{ return (loaded_modules.size() > 0); }

void PrintKnownFiles()
{
	for (auto& it : all_files)
	{
		plog::infomore(" %s (%i)\n", it.first.c_str(), it.second.locations.size());
	}
}

void PrintLoadedModules()
{
	for (auto& it : loaded_modules)
	{
		plog::infomore(" %s (%s)\n", it.get_name().c_str(), it.get_path().c_str());
	}
}

bool NewSaveFile(const char* savename)
{
	// The new save file
	Module newfile;
	newfile.set_name(savename);

	// Figure out where the save file belongs
	fs::path new_save_path = fs::u8path(sys::getUserPath()) / SAVE_FOLDER / (string(savename) + MODULE_SAVE_EXT);

	// Does it already exist?
	if (fs::exists(new_save_path))
	{
		plog::error("There is already a save file with the name '", savename, "'.\n");
		return false;
	}
	newfile.set_path(new_save_path.u8string().c_str());

	// Now we need to create a 'module.xml' for the save file.
	xml_document doc1;
	xml_node root = doc1.append_child("mod_info");
	root.append_child("name").text() = savename;
	root.append_child("version").text() = "0.0.0";
	root.append_child("author").text() = "";
	root.append_child("description").text() = "";
	root.append_child("category").text() = "";
	root.append_child("load_priority").text() = "0";

	// Open the new save file's archive and save 'module.xml' into it.
	newfile.get_archive()->open(new_save_path.u8string().c_str());
	OutFile ofile1;
	doc1.save(ofile1);
	ofile1.write_mem_to(newfile.get_archive(), "module.xml");

	// Now we need a 'load_order.xml'.
	xml_document doc2;
	root = doc2.append_child("load_order");
	OutFile ofile2;
	doc2.save(ofile2);
	ofile2.write_mem_to(newfile.get_archive(), "load_order.xml");

	newfile.close();
	return true;
}

bool LoadSaveFile(Module* savefile)
{
	// 1) Read the loadorder.xml for the save file
	// 2) Load the modules that we find, warn the user about missing/misversioned ones
	// 3) Copy the save file to a temp location
	// 4) Save file in temp location becomes 'active module'.

	// load_order.xml
	InFile loadorderxml;

	// Open load_order.xml
	Archive* a = savefile->get_archive();
	if (a->is_open())
	{
		size_t size;
		char* ptr = a->extract_data(LOAD_ORDER_FILENAME, NULL, &size, NULL, true);
		if (ptr)
		{
			loadorderxml.open(ptr, size);
		}
	}
	else
	{
		fs::path fullpath = fs::u8path(savefile->get_path()) / LOAD_ORDER_FILENAME;
		if (fs::exists(fullpath))
		{
			loadorderxml.open(fullpath);
		}
	}

	// If it doesn't exist, we spit out an error.
	if (loadorderxml.is_open() == false)
	{
		plog::error("In sys::filesystem::load_save_file():\n");
		plog::errmore("Could not find 'load_order.xml' for save file '%s'.\n", savefile->get_name().c_str());
		return false;
	}

	// Read and parse load_order.xml.
	string contents = loadorderxml.contents();
	xml_document doc;
	xml_node root;
	xml_parse_result result = doc.load_buffer_inplace(&contents[0], contents.size());
	if (!result || (root = doc.child("load_order")) == NULL)
	{
		plog::error("In sys::filesystem::load_save_file():\n");
		plog::errmore("Failed to parse 'load_order.xml' for save file '%s'.\n", savefile->get_name().c_str());
		return false;
	}

	// Clear out any existing files and references.
	Shutdown();
	Init();

	// Go through load_order.xml and find the modules that it references.
	for (xml_node node = root.child("module"); node; node = node.next_sibling("module"))
	{
		// The name of the module.
		const char* myname = readXML(node, "name", "");

		Module mod;
		mod.set_name(myname);

		// If the mod is disabled, we just skip it entirely.
		bool enabled = readXML(node, "enabled", false);
		if (!enabled)
			continue;

		// Get the path to the module, and check to see that it exists.
		fs::path modpath = fs::u8path(sys::getUserPath()) / DATA_FOLDER / (string(myname) + MODULE_EXTENSION);
		mod.set_path(modpath.u8string());
		if (!fs::exists(modpath) || !mod.load_mod_info())
		{
			modpath = fs::u8path(sys::getInstallPath()) / DATA_FOLDER / (string(myname) + MODULE_EXTENSION);
			mod.set_path(modpath.u8string());
			if (!fs::exists(modpath) || !mod.load_mod_info())
			{
				plog::error("In sys::filesystem::load_save_file(), loading save '%s':\n", savefile->get_name().c_str());
				plog::errmore("Referenced module '%s' does not exist or is invalid.\n", myname);
				continue;
			}
		}

		// We found the mod and it exists, so we add it to the list.
		loaded_modules.push_back(mod);
		load_module(&loaded_modules.back());
	}

	// Copy the save file to our temp location to be our active module.
	fs::path active_path = fs::u8path(sys::getUserPath()) / ACTIVE_MODULE_PATH;
	fs::copy_file(fs::u8path(savefile->get_path()), active_path);
	active_module.set_path(active_path.u8string());
	load_module(&active_module);

	return true;
}

void SaveGame(const char* savename)
{
	// Copy active module to savename's location.
}

InFile LoadSingleFile(const char* path, std::ios::openmode mode)
{
	if (all_files.count(path) == 0)
		return InFile();

	InFile file;

	FileEntry entry = all_files[path];

	// When looking for a single file, look through the list in REVERSE order.
	// Note that while this looks like a for loop, it should only ever look at a single element.
	// It'll only ever consider more than one if there's some error loading the first file it finds.
	for (auto mod = entry.locations.rbegin(); mod != entry.locations.rend(); ++mod)
	{
		Archive* a = (*mod)->get_archive();

		if (a->is_open())
		{
			size_t size;
			char* ptr = a->extract_data(path, NULL, &size, NULL, true);
			if (ptr)
			{
				file.open(ptr, size);
				return file;
			}
		}
		else
		{
			fs::path fullpath = fs::u8path((*mod)->get_path()) / path;
			if (fs::exists(fullpath))
			{
				file.open(fullpath, mode);
				return file;
			}
		}
	}

	return file;
}

void LoadAllFiles(const char* path, vector<InFile>& files, ios::openmode mode)
{
	if (all_files.count(path) == 0)
		return;

	files.clear();

	FileEntry entry = all_files[path];

	// Look through the list in forward order.
	// The idea here is that mods with lower load priority will have their files appear earlier in the list,
	// and changes/data that they add will likely be overwritten/modified by files from modules with higher priority.
	for (auto mod : entry.locations)
	{
		Archive* a = mod->get_archive();

		if (a->is_open())
		{
			size_t size;
			char* ptr = a->extract_data(path, NULL, &size, NULL, true);
			if (ptr)
			{
				files.emplace_back(ptr, size);
			}
		}
		else
		{
			fs::path fullpath = fs::u8path(mod->get_path()) / path;
			if (fs::exists(fullpath))
			{
				files.emplace_back(fullpath, mode);
			}
		}
	}


	return;
}

void LoadEverythingInFolder(const char* folder, vector<vector<InFile>>& files, std::vector<const char*>& paths, std::ios::openmode mode)
{
	auto lb = all_files.lower_bound(folder);
	auto ub = all_files.upper_bound(folder);

	if (lb == all_files.end())
	{
		return;
	}

	files.clear();
	paths.clear();

	for (auto it = lb; it != ub; ++it)
	{
		FileEntry this_entry = it->second;
		const char* this_path = it->first.c_str();

		files.emplace_back();
		for (auto mod : this_entry.locations)
		{
			Archive* a = mod->get_archive();

			if (a->is_open())
			{
				size_t size;
				char* ptr = a->extract_data(this_path, NULL, &size, NULL, true);
				if (ptr)
				{
					files.back().emplace_back(ptr, size);
				}
			}
			else
			{
				fs::path fullpath = fs::u8path(mod->get_path()) / this_path;
				if (fs::exists(fullpath))
				{
					files.back().emplace_back(fullpath, mode);
				}
			}
		}
		paths.push_back(this_path);
	}
}

} // namespace filemanager