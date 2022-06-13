#include "paths.h"

#ifdef MODEL_CONVERTER
#include "mc_appconfig.h"
#else
#include "appconfig.h"
#endif

#include "tools/stringhelper.h"

#ifdef PLATFORM_WIN32
#include <Windows.h>
#include <ShlObj.h> // This header screws up in an anonymous namespace, so these functions are 'static'.

static std::string init_user_dir()
{
	// Find the directory "C:/Users/%USERNAME%/AppData/Roaming/"
	wchar_t path[MAX_PATH] = {};
	SHGetFolderPath(nullptr,
		CSIDL_FLAG_CREATE | CSIDL_APPDATA,
		nullptr,
		0,
		path);

	// Append the company name and application name to the path,
	// creating any directories that don't exist yet.
	std::wstring folderpath = path;
	folderpath += L"\\"; folderpath += utf8_to_utf16(app::company_name);
	CreateDirectory(folderpath.c_str(), nullptr);
	folderpath += L"\\"; folderpath += utf8_to_utf16(app::name);
	CreateDirectory(folderpath.c_str(), nullptr);
	folderpath += L"\\";

	return utf16_to_utf8(folderpath);
}

static std::string init_install_dir()
{
	// No install path yet, we're just using the current working directory for now.
	return "";
}
#endif // PLATFORM_WIN32

#ifdef PLATFORM_SDL
#include <SDL.h>

static string init_user_dir()
{
	return SDL_GetPrefPath(app::company_name, app::name);
}

static string init_install_dir()
{
	return "";
}
#endif // PLATFORM_SDL

namespace {

std::string user_dir = init_user_dir();
std::string install_dir = init_install_dir();

} // namespace <anon>

namespace sys {

const std::string& getUserPath()
{
	return user_dir;
}

const std::string& getInstallPath()
{
	return install_dir;
}

} // namespace sys