#ifndef HVH_WC_SCENE_ENTITYNAME_H
#define HVH_WC_SCENE_ENTITYNAME_H

#include "entity.h"

#include <vector>

#include "tools/fixedstring.h"

/*
The 'name' component stores the names of each entity (if it has one) as well as a look-up table
which allows a user to search for an entity or group of entities by their name.
*/

namespace entity {
namespace name {

	/* Sets the name of the indicated entity. */
	void set(ID id, FixedString<32> entity_name);
	/* Gets the name of the indicated entity. */
	std::string get(ID id);
	/* Returns whether the indicated entity has a name. */
	bool has(ID id);
	/* Removes the name of the indicated entity. */
	void remove(ID id);

	/* Finds an entity with the given name and returns its ID. */
	/* If multiple entities share the name, which one is returned is undefined. */
	entity::ID Find(FixedString<32> entity_name);

	/* Finds all entities with the given name and returns their IDs. */
	std::vector<ID> FindAll(FixedString<32> entity_name);

	/* Initializes the Lua interface for the Name component. */
	void InitLua();

	/* Returns the number of bytes used by a given number of entity's names; Used for memory allocations. */
	constexpr size_t size_of(size_t count)
		{ return count * sizeof(FixedString<32>); }

	/* Takes a pointer to memory allocated by the scene and allocates sections of it for our purposes. */
	/* Returns a pointer to the memory immediately after what we used, which can be sent to the next component's allocation. */
	void* Allocate(void* memory, size_t capacity);

	/* Destroys everything. */
	void Clear();

}} // namespace component::name

#endif // HVH_WC_SCENE_ENTITYNAME_H