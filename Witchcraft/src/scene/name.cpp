#include "name.h"

#include "component.h"

#include <unordered_map>
using namespace std;

#include "tools/fixedstring.h"

namespace entity {
namespace name {

namespace {

	unordered_map<ID, size_t> map;
	unordered_multimap<FixedString<32>, ID> lookup;

	FixedString<32>* names = nullptr;

	size_t capacity = 0;
	size_t entries = 0;

} // namespace <anon>

void* Allocate(void* mem, size_t cap)
{
	if (capacity != 0)
	{
		plog::fatal("Programmer Error; cannot Allocate a component which has already been Allocated!");
		return nullptr;
	}

	capacity = cap;
	map.reserve(cap);
	lookup.reserve(cap);

	names = (FixedString<32>*)mem;
	return (void*)(names + capacity);
}

void Clear()
{
	names = nullptr;

	capacity = 0;
	entries = 0;

	map.clear();
	lookup.clear();
}

void set(ID id, FixedString<32> entity_name)
{
	if (has(id))
	{
		size_t index = map[id];

		// First we have to get our old name, and find it in the lookup map.
		FixedString<32> oldname = names[index];
		auto range = lookup.equal_range(oldname);
		for (auto it = range.first; it != range.second; ++it)
		{
			// Once we find it,
			if (it->second == id)
			{
				// Erase it and break out of the loop.
				lookup.erase(it);
				break;
			}
		}

		// Then we can add the new name!
		names[index] = entity_name;
		lookup.insert({ entity_name, id });
	}
	else
	{
		size_t index = entries;
		if (index >= capacity) { plog::error("Cannot add another Name; out of memory!"); return; }
		entries++;

		map[id] = index;
		names[index] = entity_name;
		lookup.insert({ entity_name, id });
	}
}

string get(ID id)
{
	if (!has(id)) return "";
	else return names[map[id]].c_str;
}

bool has(ID id)
	{ return (map.count(id) > 0); }

void remove(ID id)
	{ plog::error("entity::<component>::remove() is deprecated.  Components should only be added during initialization, and removed during de-initialization."); }

ID Find(FixedString<32> entity_name)
{
	auto range = lookup.equal_range(entity_name);
	if (range.first == lookup.end())
		return {};
	else
		return range.first->second;
}

vector<ID> FindAll(FixedString<32> entity_name)
{
	auto range = lookup.equal_range(entity_name);
	if (range.first == lookup.end())
		return {};
	else
	{
		vector<ID> result;
		for (auto it = range.first; it != range.second; ++it)
			{ result.push_back(it->second); }
		return result;
	}
}


}} // namespace component::name