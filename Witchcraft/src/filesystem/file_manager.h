#ifndef HVH_WC_FILESYSTEM_FILEMANAGER_H
#define HVH_WC_FILESYSTEM_FILEMANAGER_H

#include <stdint.h>
#include <vector>
#include "module.h"
#include "file.h"

/*
When the game starts:
First we initialize and load the CORE module.
Next we initialize the TEMP module.
Next we look through the USER save folder and find every save file there.
Then we look through the USER data folder to find every module there.
Then we look through the INSTALL data folder to find every module which does not share a name with one already loaded.
The modules that we find will be opened and have their "module.xml" file loaded and stored.

Then, the main menu is displayed.
The user chooses between New Game, Load Game, Options, etc.
If the user chooses New Game, we load "load_order.xml" from the main user directory.  This mirrors the most recently saved "load_order.xml".
If the user chooses LoadGame, they're given a list of files.  When the user selects one, we load "load_order.xml" from that save file archive.
Using load_order.xml, we build a sorted list of every available module and whether or not they're enabled.
Dependancy/conflict resolution happens at this stage.
The user is given the chance to rearrange the list how they see fit, and to enable/disable modules.
Once the user has finished and has chosen to play the game, we go through our list of modules.
Modules which are enabled will have their file lists added to our own.
Modules which are disabled will be closed (we don't care if the user deletes a mod we aren't using mid-game).

Throughout gameplay, various files will be saved to TEMP.  When a file is loaded, files in TEMP get highest priority.
When the user saves their game, we merge TEMP with their save file.  If they quit without saving, any files in TEMP are deleted.

Future note:
If the user chooses Multiplayer from the main menu, they're given means to choose a server to connect to.
If they choose to connect to that server, they'll download a load_order.xml from it.
Any modules referenced in this load_order.xml which aren't in our list or are a different version
will be downloaded from the server and saved in a folder named after that server's location.
In case of a name conflict, these server modules will overwrite the local ones.

When the user returns to the main menu, we need the means to unload any modules form the previous play session,
and restart from the "look through data folders to find available modules" stage.

Load priority:

TEMP (highest)
The active save file
The loaded modules, sorted according to the user-defined load order
CORE (lowest)

THE ABOVE NOTES ARE OUTDATED
TODO: FIGURE OUT WTF WE'RE ACTUALLY DOING NOW, JFC

*/



namespace filemanager
{
	/* Initializes the filesystem and loads the EngineData module into the list of known files. */
	/* Returns false if initialization fails. */
	bool Init();
	/* Cleans up the filesystem. */
	void Shutdown();

	/* Returns whether or not the filesystem has been initialized. */
	bool isInitialized();

	/*
	std::vector<Module*> GetLoadOrder(int savefile = -1);
	void LoadModules(std::vector<Module*>& order, Module* active = NULL);

	// These functions are meant to be used to iterate over our collection of modules.
	// They return NULL if 'i' is out of bounds.
	unsigned int num_available_modules();
	Module* get_available_module(unsigned int i);
	Module* get_loaded_module(unsigned int i);
	Module* get_available_save(unsigned int i);
	*/

	void PrintKnownFiles();
	void PrintLoadedModules();

	bool NewSaveFile(const char* name);
	bool LoadSaveFile(Module* savefile);
	void SaveGame(const char* name);

	// These functions are the whole reason we're doing any of this.
	// They look for the requested file in all of our loaded modules.
	InFile LoadSingleFile(const char* path, std::ios::openmode mode = 0);
	void LoadAllFiles(const char* path, std::vector<InFile>& files, std::ios::openmode mode = 0);
	void LoadEverythingInFolder(const char* path, std::vector<InFile>& files, std::vector<std::string>& paths, std::ios::openmode mode = 0);

	void SaveFileToActive(const char* path, OutFile& file);
//	void SaveActiveModule();
}

#endif