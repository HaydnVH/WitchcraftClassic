#ifndef HVH_WC_SCENE_COMPONENT_H
#define HVH_WC_SCENE_COMPONENT_H

#include "entity.h"
#include "tools/structofarrays.h"
#include "sys/printlog.h"
#include <unordered_map>

template <typename... Ts>
class ComponentTable
{
public:

	void AddEntry(entity::ID id, const Ts&... args)
	{
		// Don't add a component to an entity which already has that component.
		if (hasEntry(id))
			return;

		redirect[id] = (uint32_t)soa.size();
		soa.push_back(id, args...);
	}

	void RemoveEntry(entity::ID id)
	{
		if (!hasEntry(id))
			return;

		uint32_t index = redirect[id];

		entity::ID old_back = soa.get<0>().back();
		redirect[old_back] = index;
		redirect.erase(id);

		soa.erase_swap(index);
	}

	bool hasEntry(entity::ID id)
	{
		if (redirect.count(id) <= 0 || redirect[id] == 0 || redirect[id] >= soa.size())
			return false;
		else
			return true;
	}

//protected:
	ComponentTable(const Ts&... default_state)
		{ soa.push_back(0, default_state...); }
	~ComponentTable() {};

	StructOfArrays<entity::ID, Ts...> soa;

	uint32_t index(entity::ID id)
	{
		if (hasEntry(id) == false)
		{
			plog::error("Attempting to obtain component index for invalid entity.\n");
			return 0;
		}
		else
			return redirect[id];
	}

private:
	std::unordered_map<entity::ID, uint32_t> redirect;
//	std::vector<uint32_t> redirect;
};

#endif // HVH_WC_SCENE_COMPONENT_H