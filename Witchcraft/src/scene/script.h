#ifndef HVH_WC_SCENE_SCRIPT_H
#define HVH_WC_SCENE_SCRIPT_H

#include "entity.h"

#include "tools/fixedstring.h"

#include <rapidjson/document.h>


/*
The 'script' component keeps track of which scripts are attached to a given entity,
and is responsible for calling the 'on_init' and 'on_destruct' functions in those scripts.
*/

namespace entity {
namespace script {

	/* Attach a script to an entity, calling the 'on_init()' function in the script if present. */
	void attach(ID id, FixedString<32> scriptname, rapidjson::Value::ConstMemberIterator parameters = {});
	/* Detach a script from an entity, calling the 'on_destruct()' function in the script if present. */
	/* Doesn't actually remove the script from the entity's map; this function should not be used by outside code. */
	void detach(ID id, FixedString<32> scriptname);
	/* Detaches all scripts from an entity. */
	void detachAll(ID id);
	/* Returns whether or not an entity has the indicated script attached. */
	bool has(ID id, FixedString<32> scriptname);

	/* Initializes the Lua interface for the Script subsystem. */
	/* This function is called once during program initialization, and should not be called again. */
	void InitLua();

	/* Returns the number of bytes used by a given number of entity's scripts; Used for memory allocations. */
	constexpr size_t size_of(size_t count)
		{ return 0; }

	/* Takes a pointer to memory allocated by the scene and allocates sections of it for our purposes. */
	/* Returns a pointer to the memory immediately after what we used, which can be sent to the next component's allocation. */
	void* Allocate(void* memory, size_t capacity);

	/* Destroys everything. */
	void Clear();

}} // namespace entity::script


#endif // HVH_WC_SCENE_SCRIPT_H