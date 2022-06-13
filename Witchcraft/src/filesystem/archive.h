#ifndef HVH_WC_FILESYSTEM_ARCHIVE_H
#define HVH_WC_FILESYSTEM_ARCHIVE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

/*
Archives are arranged on disc in 3 parts, or 'chunks'.

The first chunk is the archive header.  It contains information about the archive itself,
such as how many files are stored in it, the total size of the file, and where the dictionary is stored.

The second part is the actual file data itself.  The order in which files are stored in this chunk is "undefined".

The third part is the dictionary, which consists of two separate lists.

The first list is the full path of every file stored within the archive, sorted by alphebetical order.
This list is easy to search using a simple binary search to find the file you want.
The second list contains information about the file, such as its offset and size.
The second list is sorted according to the first list.  So, if you find the file you're looking for in the first list,
you can use that iterator to go straight to the file's information in the second list.

All file paths are stored in UTF-8, with forward slash path separators (/, not \).
File paths are stored in a fixed-size array that's always 64 bytes long.  The 64th byte is reserved for the null terminator,
leaving you with no more than 63 bytes of space to store the file path.

The order of the actual file data is actually determined by the order in which files are added to the archive.
The archive cannot be sorted, but you can create a brand new archive from scratch in such a way that files are added in alphabetical order,
which will cause files that are in the same folder to be next to each other in the archive, potentially speeding up read times.
The order of the file data should never be relied upon, and should be treated as undefined.

When an archive is opened, it's header and dictionary are loaded into memory, but the file data itself is not.  This data is only touched when it's requested.
*/

enum ArchiveEnum
{
	ARCHIVE_DO_NOT_REPLACE = 0,
	ARCHIVE_REPLACE,
	ARCHIVE_REPLACE_IF_NEWER
};

constexpr const int ARCHIVE_FILEPATH_FIXED_SIZE = 64;
constexpr const int ARCHIVE_FILEPATH_MAX_STRLEN = 63;


struct FixedFilePath
{
	char path[ARCHIVE_FILEPATH_FIXED_SIZE];

	inline bool operator == (const char* rhs)
	{
		return strncmp(path, rhs, ARCHIVE_FILEPATH_MAX_STRLEN) == 0;
	}

	inline bool operator != (const char* rhs)
	{
		return !(*this == rhs);
	}

	inline bool operator < (const char* rhs)
	{
		return strncmp(path, rhs, ARCHIVE_FILEPATH_MAX_STRLEN) < 0;
	}

	inline bool operator < (const FixedFilePath& rhs)
	{
		return strncmp(path, rhs.path, ARCHIVE_FILEPATH_MAX_STRLEN) < 0;
	}
};

class Archive
{
public:
	Archive()
		: header({}),
		path_list(NULL),
		info_list(NULL),
		file(NULL),
		saved_path(NULL),
		was_modified(false),
		files_were_deleted(false)
	{}

	// Archive::Open() finds an archive on disc and opens it, filling out the Archive's header and dictionary as appropriate.
	bool open(const char* utf8path);

	// Archive::Close() closes the archive, saving any modifications to disc.
	void close();

	// Archive::Rebuild() rebuilds the archive, sorting the data according to filename and erasing any unreferenced blocks.
	void rebuild();

	// Archive::is_open() simply returns whether or not the archive in question is open/valid or not.
	bool is_open()
	{
		return (file != NULL);
	}

	// Archive::FileExists() checks to see if a file exists in the archive.
	bool file_exists(const char* utf8path);

	// Archive::ExtractData() finds the file (utf8filename) and extracts it to an in-memory buffer.
	// if alloc is true, extractData will allocate a buffer on the heap which must be freed using "delete[]".
	char* extract_data(const char* utf8filename, char* ptr, size_t* size, __int64* timestamp, bool alloc = false);

	// Archive::ExtractFile() finds the file (utf8filename) and extracts it to a file on-disc (utf8destpath).
	void extract_file(const char* utf8filename, const char* utf8destpath);

	// Archive::InsertData() takes a region of memory and, treating it like a single contiguous file, inserts it into the archive.
	bool insert_data(const char* utf8filename, char* ptr, size_t size, __int64 timestamp, uint8_t replace = ARCHIVE_REPLACE_IF_NEWER);

	// Archive::InsertFile() opens a file on disc (utf8srcpath) and inserts it's entire contents into the archive.
	bool insert_file(const char* utf8filename, const char* utf8srcpath, uint8_t replace = ARCHIVE_REPLACE_IF_NEWER);

	// Archive::Erase() erases a file from the archive.
	// All it really does is remove the file's information from the dictionary, and flag the class instance as dirty.
	// When the archive is closed, if one or more files have been deleted, the archive is rebuilt.
	int erase_file(const char* utf8filename);

	// Archive::Pack() searches a folder (specified by 'utf8path') recursively and adds every file found to the archive.
	void pack(const char* utf8path, uint8_t replace = ARCHIVE_REPLACE_IF_NEWER);

	// Archive::Unpack() extracts every file in the archive and saves them to the location specified by 'utf8path'.
	void unpack(const char* utf8path);

	// Archive::Merge() opens an archive (utf8otherpath) and inserts all of its files into this archive.
	void merge(const char* utf8otherpath, uint8_t replace = ARCHIVE_REPLACE_IF_NEWER);

	uint32_t num_files()
	{
		return header.num_files;
	}

	const std::vector<FixedFilePath>& file_list()
	{
		return path_list;
	}

private:

	struct Header
	{
		char magic[8]; // This is set to a predefined string and used to ensure that the file type is correct.
		uint64_t back; // This points to the beginning of the dictionary, and defines the file data region.
		uint32_t flags;
		uint32_t num_files;
		uint16_t version; // Which version of this software was used to create the archive?
		char _reserved[38]; // Reserved in case we need it for future versions without having to break compatability.
	};

	struct FileInfo
	{
		uint64_t offset; // Where, relative to the beginning of the archive, is the file located?
		uint64_t size_compressed; // How many bytes in the archive itself does the file take?
		uint64_t size_uncompressed; // How many bytes large will the file be after we decompress it?  If this if equal to 'size_compressed', the file is uncompressed.
		int64_t timestamp; // When was the file created before we added it to the archive?
		uint32_t flags;
		char _reserved[28]; // Reserved for future use.  May or may not actually use.
	};

	Header header;

	std::vector<FixedFilePath> path_list;
	std::vector<FileInfo> info_list;

	//	FixedFilePath* path_list;
	//	FileInfo* info_list;

	FILE* file;
	const char* saved_path;
	bool was_modified;
	bool files_were_deleted;
};






#endif //HVH_WC_FILESYSTEM_ARCHIVE_H
