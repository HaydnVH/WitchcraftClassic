#include "archive.h"

// We use the filesystem library internally to make things easier for us.
// On a platform where the filesystem library isn't available, we would
// use whatever functions the operating system makes available.
// We don't put <filesystem> into archive.h because newer C++ libraries may not be available everywhere,
// and keeping our interface ignorant of it will help portability to platforms that don't support it.
#include <filesystem>
namespace fs = std::filesystem; // Just a shortcut, typing "fs::" is faster than "std::tr2::sys::".

#include <algorithm>
using namespace std;

constexpr const char* ARCHIVE_MAGIC = "WC_ARCV";
constexpr const uint16_t ARCHIVE_CURRENT_VERSION = 1;

// Gotta handle 64-bit file offsets
#ifdef _WIN32
#define fseek(file,offset,origin) _fseeki64(file,offset,origin)
#define fopen_w(filename) _wfopen(filename, L"wb")
#define fopen_rw(filename) _wfopen(filename, L"r+b")
#define fopen_r(filename) _wfopen(filename, L"rb")
#else
#define fopen_w(filename) fopen(filename, "wb")
#define fopen_rw(filename) fopen(filename, "r+b")
#define fopen_r(filename) fopen(filename, "rb")
#endif


bool Archive::open(const char* utf8path)
{
	// If we're already open, close ourselves first.
	if (file)
		close();

	// On Windows, this line converts the utf8 string to utf16.
	fs::path archive_path = fs::u8path(utf8path);

	// If the archive we're trying to open doesn't exist...
	if (!fs::exists(archive_path))
	{
		// Make sure the parent path for the archive exists,
		if (!fs::exists(archive_path.parent_path()))
		{
			// Create it if it doesn't.
			fs::create_directories(archive_path.parent_path());
		}

		// If the file doesn't exist yet, then we can't open it with "r+", so we create it here.
		file = fopen_w(archive_path.c_str());

		Archive::Header new_header = {};
		strncpy(new_header.magic, ARCHIVE_MAGIC, 8);
		new_header.back = sizeof(Archive::Header);
		new_header.version = ARCHIVE_CURRENT_VERSION;

		fwrite(&new_header, sizeof(Archive::Header), 1, file);
		fclose(file);
		was_modified = true;
	}

	// "r+" opens the file for both reading and writing and keeps existing contents.
	file = fopen_rw(archive_path.c_str());

	if (file == NULL)
	{
		fprintf(stderr, "Archive '%s' could not be opened.\n", utf8path);
		return false;
	}

	// Load the header
	fread(&header, sizeof(Archive::Header), 1, file);

	// Check the header to make sure the file format is valid.
	if (strncmp(header.magic, ARCHIVE_MAGIC, 8) != 0)
	{
		fprintf(stderr, "Archive '%s' is not a valid archive.\n", utf8path);
		fclose(file);
		file = NULL;
		return false;
	}

	if (header.num_files > 0)
	{
		// Reserve size for the dictionary
		path_list.resize(header.num_files);
		info_list.resize(header.num_files);

		// Seek to it's position,
		fseek(file, header.back, SEEK_SET);

		// Read into the arrays.
		fread(path_list.data(), sizeof(FixedFilePath), header.num_files, file);
		fread(info_list.data(), sizeof(Archive::FileInfo), header.num_files, file);
	}

	saved_path = utf8path;
	return true;
}


void Archive::close()
{
	// If the file isn't open, we have nothing to close.
	if (file == NULL)
		return;

	// If the archive has been modified, we update the file's header and dictionary.
	if (was_modified)
	{
		// Update the archive header.
		fseek(file, 0, SEEK_SET);
		fwrite(&header, sizeof(Archive::Header), 1, file);

		if (files_were_deleted)
		{
			// If we deleted or overwrote anything in the archive, we should rebuild it.
			rebuild();
		}

		// We'll have to re-write the archive dictionary.
		if (header.num_files > 0)
		{
			fseek(file, header.back, SEEK_SET);
			fwrite(path_list.data(), sizeof(FixedFilePath), header.num_files, file);
			fwrite(info_list.data(), sizeof(Archive::FileInfo), header.num_files, file);
		}
	}

	// Finally close the file.
	fclose(file);
	file = NULL;

	was_modified = false;
	files_were_deleted = false;
}


void Archive::rebuild()
{
	if (file == NULL)
		return;

	fs::path temppath = fs::u8path(std::string(saved_path) + "_TEMP");

	FILE* tempfile;
	tempfile = fopen_w(temppath.c_str());

	// First we write the header to the new archive.
	fwrite(&header, sizeof(Archive::Header), 1, tempfile);
	uint64_t newback = sizeof(Archive::Header);

	// Next we go through all of our files.
	for (uint32_t i = 0; i < header.num_files; ++i)
	{
		// Allocate a buffer large enough to store the file.
		char* buffer = new char[(size_t)info_list[i].size_compressed];

		// Read the file into memory.
		fseek(file, info_list[i].offset, SEEK_SET);
		fread(buffer, 1, (size_t)info_list[i].size_compressed, file);

		// Write the file to the temporary archive.
		fseek(tempfile, newback, SEEK_SET);
		fwrite(buffer, 1, (size_t)info_list[i].size_compressed, tempfile);

		// Keep track of where the file is now
		info_list[i].offset = newback;
		newback += info_list[i].size_compressed;

		// Clear up the memory we used.
		delete[] buffer;
	}

	// Now we write the dictionary to the temporary archive.
	fwrite(path_list.data(), sizeof(FixedFilePath), header.num_files, tempfile);
	fwrite(info_list.data(), sizeof(Archive::FileInfo), header.num_files, tempfile);

	// Since our 'back' has changed, we need to correct it and re-write the header.
	header.back = newback;
	fseek(tempfile, 0, SEEK_SET);
	fwrite(&header, sizeof(Archive::Header), 1, tempfile);

	// Close both archives
	fclose(file);
	fclose(tempfile);

	// Have the filesystem clean things up for us.
	fs::path mypath = fs::u8path(saved_path);
	fs::rename(temppath, mypath);

	// Re-open the file.
	file = fopen_rw(mypath.c_str());

}

int Archive::erase_file(const char* path)
{
	if (file == NULL)
		return -1;

	// Find the index of the file we're looking to erase.
	auto it = lower_bound(path_list.begin(), path_list.end(), path);
	if (it == path_list.end() || *it != path)
		return -1;

	size_t file_index = it - path_list.begin();

	// Erase the file's entry from the dictionary.
	path_list.erase(it);
	info_list.erase(info_list.begin() + file_index);
	header.num_files--;

	was_modified = true;
	files_were_deleted = true;

	return (int)file_index;
}

bool Archive::file_exists(const char* path)
{
	if (file == NULL)
		return false;

	// Find the index of the file we're looking for.
	auto it = lower_bound(path_list.begin(), path_list.end(), path);
	if (it == path_list.end() || *it != path)
		return false;
	else
		return true;
}

char* Archive::extract_data(const char* path, char* ptr, size_t* size, __int64* timestamp, bool alloc)
{
	if (file == NULL)
	{
		*size = 0;
		return NULL;
	}

	// Search for the file we're looking for.
	auto it = lower_bound(path_list.begin(), path_list.end(), path);
	if (it == path_list.end() || *it != path)
	{
		*size = 0;
		return NULL;
	}

	size_t file_index = it - path_list.begin();

	FileInfo info = info_list[file_index];
	*size = (size_t)info.size_uncompressed;
	if (timestamp) *timestamp = info.timestamp;

	if (!ptr && !alloc)
	{
		// Found the file, but we don't have any space allocated for it, and they don't want us allocating space.
		return NULL;
	}

	fseek(file, info.offset, SEEK_SET);

	if (feof(file))
	{
		// File's offset points to beyond the end of the archive.  Somehow.
		*size = 0;
		return NULL;
	}

	// If we haven't been given a pointer, and we have permission to allocate, then we allocate a buffer.
	if (!ptr && alloc)
	{
		ptr = new char[(size_t)info.size_uncompressed];
	}

	// If the file's compressed size is equal to it's uncompressed size, the file is not compressed.
	if (info.size_compressed == info.size_uncompressed)
	{
		// Load the file contents straight into memory.
		fread(ptr, 1, (size_t)info.size_uncompressed, file);
	}
	else
	{
		// TODO: Handle file decompression
		if (alloc)
			delete[] ptr;
		return NULL;
	}

	// Finally, we return the ptr we just loaded the memory to.
	return ptr;
}


void Archive::extract_file(const char* filepath, const char* dest)
{
	size_t size = 0;
	__int64 timestamp = 0;
	char* ptr = extract_data(filepath, NULL, &size, &timestamp, true);

	if (ptr)
	{
		fs::path destpath = fs::u8path(dest);
		std::error_code ec;
		fs::create_directories(destpath.parent_path(), ec);
		if (ec)
		{
			fprintf(stderr, "Filesystem error:%s\n", ec.message().c_str());
			return;
		}

		FILE* destfile = fopen_w(destpath.c_str());

		if (destfile)
		{
			fwrite(ptr, 1, size, destfile);
			fclose(destfile);

			// Correct the file's 'last write time'
			// TODO: Make sure that this is actually working with the type/unit conversions!
			fs::last_write_time(dest, fs::file_time_type(chrono::seconds(timestamp)));
		}

		delete[] ptr;
	}
}

bool Archive::insert_data(const char* path, char* ptr, size_t size, __int64 timestamp, uint8_t replace)
{
	if (file == NULL)
		return false;

	// Copy the file's path and convert backslashes to forward slashes.
	// Functions like extract and exists will simply fail if given backslashes.
	FixedFilePath newpath = {};
	strncpy(newpath.path, path, ARCHIVE_FILEPATH_MAX_STRLEN);
	for (int i = 0; i < ARCHIVE_FILEPATH_MAX_STRLEN && newpath.path[i] != '\0'; ++i)
	{
		if (newpath.path[i] == '\\')
			newpath.path[i] = '/';
	}

	// Create a dictionary entry for the new file.
	FileInfo newinfo = {};
	newinfo.offset = header.back;
	newinfo.size_compressed = size;
	newinfo.size_uncompressed = size;
	newinfo.timestamp = timestamp;

	// Perform a binary search to find the file we're looking for
	auto it = lower_bound(path_list.begin(), path_list.end(), path);
	if (it == path_list.end() || *it != path)
	{
		// The file is not already in the archive, so first thing's first we insert it into the dictionary.
		if (it == path_list.end())
		{
			path_list.push_back(newpath);
			info_list.push_back(newinfo);
		}
		else
		{
			size_t file_index = it - path_list.begin();
			path_list.insert(it, newpath);
			info_list.insert(info_list.begin() + file_index, newinfo);
		}
		header.num_files++;
	}
	else
	{
		size_t file_index = it - path_list.begin();

		// The specified file is already in this archive, so use 'replace' to decide what to do.
		if (replace == ARCHIVE_DO_NOT_REPLACE)
			return false;
		else if (replace == ARCHIVE_REPLACE)
		{
			files_were_deleted = true;
			info_list[file_index] = newinfo;
		}
		else if (replace == ARCHIVE_REPLACE_IF_NEWER)
		{
			FileInfo info = info_list[file_index];
			if (timestamp > info.timestamp)
			{
				files_were_deleted = true;
				info_list[file_index] = newinfo;
			}
			else
				return false;
		}
	}

	// TODO: Handle file compression

	// If the file has a non-zero size,
	if (newinfo.size_compressed > 0)
	{
		// We write the file contents.
		fseek(file, header.back, SEEK_SET);
		fwrite(ptr, 1, (size_t)newinfo.size_compressed, file);
		header.back += newinfo.size_compressed;
	}

	was_modified = true;
	return true;
}

bool Archive::insert_file(const char* filepath, const char* src, uint8_t replace)
{
	if (strlen(filepath) > ARCHIVE_FILEPATH_MAX_STRLEN)
	{
		fprintf(stderr, "Cannot insert '%s', path too long (max %i bytes).\n", filepath, ARCHIVE_FILEPATH_MAX_STRLEN);
		return false;
	}

	// Get the file's 'last write time' and convert it into a usable integer.
	// TODO: Make sure that the units/types line up properly!
	__int64 timestamp = fs::last_write_time(src).time_since_epoch().count();

	// Open the file
	FILE* srcfile = fopen_r(fs::u8path(src).c_str());

	if (srcfile == NULL)
		return false;

	// Slurp up the file contents
	int64_t filesize = fs::file_size(src);
	char* buffer = new char[(size_t)filesize];
	size_t realsize = fread(buffer, 1, (size_t)filesize, srcfile);

	// Insert the file into the archive
	bool retval = insert_data(filepath, buffer, realsize, timestamp, replace);

	// Clean up after ourselves
	fclose(srcfile);
	delete[] buffer;

	return retval;
}

void recursive_pack(Archive& archive, const fs::path& parent, fs::path child, uint8_t replace)
{
	for (fs::directory_iterator it(parent / child); it != fs::directory_iterator(); ++it)
	{
		// Path doesn't exist.  This case should never happen, but hey, just in case!
		if (fs::exists(it->path()) == false)
			continue;

		// Path is a directory,
		if (is_directory(it->path()))
		{
			// So we need to go deeper.
			recursive_pack(archive, parent, child / it->path().filename(), replace);
		}
		else
		{
			// Path is a file, so we insert it into the archive.
			archive.insert_file((child / it->path().filename()).u8string().c_str(), it->path().u8string().c_str(), replace);
		}
	}
}

void Archive::pack(const char* src, uint8_t replace)
{
	if (file == NULL)
		return;

	fs::path srcpath = fs::u8path(src);

	if (!fs::exists(srcpath))
	{
		fprintf(stderr, "'%s' does not exist.\n", src);
		return;
	}

	if (!fs::is_directory(srcpath))
	{
		fprintf(stderr, "'%s' is not a directory.\n", src);
		return;
	}
	else
	{
		recursive_pack(*this, srcpath, "", replace);
	}
}

void Archive::unpack(const char* dst)
{
	if (file == NULL)
		return;

	fs::path dstpath = fs::u8path(dst);

	if (!fs::exists(dstpath))
	{
		create_directories(dstpath);
	}

	if (!fs::is_directory(dstpath))
	{
		fprintf(stderr, "'%s' already exists and is not a directory.\n", dst);
		return;
	}

	for (uint32_t i = 0; i < header.num_files; ++i)
	{
		FixedFilePath& entry = path_list[i];
		extract_file(entry.path, (dstpath / entry.path).u8string().c_str());
		// The unncessary conversion between utf8 and utf16 is not worth worrying about because unpack() will NEVER be run during normal gameplay.
	}
}

void Archive::merge(const char* otherpath, uint8_t replace)
{
	Archive other;
	other.open(otherpath);

	for (uint32_t i = 0; i < other.header.num_files; ++i)
	{
		FixedFilePath entry = other.path_list[i];
		size_t size;
		int64_t timestamp;
		char* ptr = other.extract_data(entry.path, NULL, &size, &timestamp, true);
		insert_data(entry.path, ptr, size, timestamp, replace);
		delete[] ptr;
	}
}