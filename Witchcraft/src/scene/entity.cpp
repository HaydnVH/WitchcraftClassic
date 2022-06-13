#include "entity.h"

#include "name.h"

#include <vector>
#include "sys/printlog.h"

namespace entity {

namespace {

uint32_t next_index;
std::vector<ID> free_list;

} // namespace entity::<anon>


ID Create(uint16_t module_id)
{
	ID result = 0;
	if (free_list.size() > 0)
	{
		result = free_list.back();
		free_list.pop_back();
	}
	else
	{
		if (next_index == 0xfffffff)
		{
			plog::error("Failed to create entity; index too large!\n");
			return result;
		}

		result = next_index;
		next_index++;
	}
	return result;
}

void Destroy(ID id)
{
	free_list.push_back(id);
}

std::string toString(ID id)
{
	if (id == 0)
		return "<NULL>";

	char str[32];
	snprintf(str, 32, "<%i>", id);

	return str;
}


} // namespace entity