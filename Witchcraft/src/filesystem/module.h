#ifndef HVH_WC_FILESYSTEM_MODULE_H
#define HVH_WC_FILESYSTEM_MODULE_H

#include <string>
#include <vector>
#include <memory>

#include "archive.h"

class Module
{
public:
	Module()
	:	version(0),
		priority(-1.0f),
		enabled(false),
		found(false),
		loaded(false)
	{}

	void open();
	void close();

	bool load_mod_info();

	bool load_file_list();
	void unload_file_list();
	bool is_archive();

	inline const std::string& get_name() const
		{ return name; }

	inline void set_name(const std::string& val)
		{ name = val; }

	inline const std::string& get_author() const
		{ return author; }

	inline const std::string& get_description() const
		{ return description; }

	inline const std::string& get_category() const
		{ return category; }

	inline const std::string& get_path() const
		{ return path; }

	inline void set_path(const std::string& val)
		{ path = val; }

	inline Archive* get_archive()
		{ return &archive; }

	inline void enable()
		{ enabled = true; }

	inline void disable()
		{ enabled = false; }

	inline bool is_enabled() const
		{ return enabled; }

	inline void set_enabled(bool val)
		{ enabled = val; }

	inline bool is_found() const
		{ return found; }

	inline void set_found(bool val)
		{ found = val; }

	inline bool is_loaded() const
		{ return loaded; }

	inline std::vector<FixedFilePath>& get_file_list()
		{ return file_list; }

	inline float load_priority()
		{ return priority; }

private:

	struct ModReference
	{
		std::string name;
		std::string note;
		union
		{
			struct
			{
				unsigned short patchver;
				unsigned char minorver;
				unsigned char majorver;
			};
			unsigned int version;
		};

		unsigned short status;
	};

	std::string name;
	std::string author;
	std::string description;
	std::string category;
	std::string path;

	union
	{
		struct
		{
			unsigned short patchver;
			unsigned char minorver;
			unsigned char majorver;
		};
		unsigned int version;
	};

	float priority;

	Archive archive;

	std::vector<FixedFilePath> file_list;
	std::vector<ModReference> dependancies;
	std::vector<ModReference> known_conflicts;

	bool enabled;
	bool found;
	bool loaded;
};

#endif