#ifndef HVH_WC_SCENE_ENTITY_H
#define HVH_WC_SCENE_ENTITY_H

#include <stdint.h>
#include <string>

namespace entity {

	typedef uint32_t ID;

	ID Create(uint16_t module_id);
	void Destroy(ID id);

	std::string toString(ID id);

} // namespace entity

#endif // HVH_WC_SCENE_ENTITY_H