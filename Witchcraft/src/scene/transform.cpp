#include "transform.h"
#include "component.h"

using namespace vmath;

#include <unordered_map>
using namespace std;

namespace entity {
namespace transform {

namespace {

	unordered_map<ID, size_t> map;

	vec3* positions = nullptr;
	quat* rotations = nullptr;
	vec3* scales = nullptr;

	vec3* prev_pos = nullptr;
	quat* prev_rot = nullptr;
	vec3* prev_scl = nullptr;

	mat4* matrices = nullptr;

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

	positions = (vec3*)mem;
	rotations = (quat*)(positions + capacity);
	scales = (vec3*)(rotations + capacity);
	prev_pos = (vec3*)(scales + capacity);
	prev_rot = (quat*)(prev_pos + capacity);
	prev_scl = (vec3*)(prev_rot + capacity);
	matrices = (mat4*)(prev_scl + capacity);
	return (void*)(matrices + capacity);
}

void Clear()
{
	positions = nullptr;
	rotations = nullptr;
	scales = nullptr;
	prev_pos = nullptr;
	prev_rot = nullptr;
	prev_scl = nullptr;
	matrices = nullptr;

	capacity = 0;
	entries = 0;
	map.clear();
}

void add(ID id, vmath::vec3 pos, vmath::quat rot, vmath::vec3 scale)
{
	if (has(id)) { plog::error("Cannot add a transform component to an entity which already has one!"); return; }

	size_t index = entries;
	if (index >= capacity) { plog::error("Cannot add another Transform; out of memory!"); return; }
	entries++;

	map[id] = index;
	positions[index] = pos;
	rotations[index] = rot;
	scales[index] = scale;
}

void remove(ID id)
	{ plog::error("entity::<component>::remove() is deprecated.  Components should only be added during initialization, and removed during de-initialization."); }

bool has(ID id)
	{ return (map.count(id) > 0); }

//// Position stuff ////

void setPos(ID id, vec3 new_pos)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	positions[map[id]] = new_pos;
}

void move(ID id, vmath::vec3 delta_pos)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	positions[map[id]] += delta_pos;
}

vmath::vec3 getPos(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return VEC3_ZERO; }
	return positions[map[id]];
}

void snapPos(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	size_t index = map[id]; prev_pos[index] = positions[index];
}

vec3 interpolatePos(ID id, float interpolation)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return VEC3_ZERO; }
	size_t index = map[id]; return vec3::lerp(prev_pos[index], positions[index], interpolation);
}

//// Rotation stuff ////

void setRot(ID id, quat new_rot)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	rotations[map[id]] = new_rot;
}

void rotate(ID id, vmath::quat delta_rot)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	rotations[map[id]] *= delta_rot;
}

vmath::quat getRot(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return QUAT_IDENTITY; }
	return rotations[map[id]];
}

void snapRot(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	size_t index = map[id]; prev_rot[index] = rotations[index];
}

quat interpolateRot(ID id, float interpolation)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return QUAT_IDENTITY; }
	size_t index = map[id]; return quat::lerp(prev_rot[index], rotations[index], interpolation);
}

//// Scale stuff ////

void setScale(ID id, vmath::vec3 new_scale)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	scales[map[id]] = new_scale;
}

vmath::vec3 getScale(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return VEC3_ONE; }
	return scales[map[id]];
}

void snapScale(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	size_t index = map[id]; prev_scl[index] = scales[index];
}

vec3 interpolateScale(ID id, float interpolation)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return VEC3_ONE; }
	size_t index = map[id]; return vec3::lerp(prev_scl[index], scales[index], interpolation);
}

//// Matrix stuff ////

mat4 getMatrix(ID id)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return MAT4_IDENTITY; }
	return matrices[map[id]];
}

void applyPreTransform(ID id, vmath::mat4 matrix)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	matrices[map[id]] *= matrix;
}

void applyPostTransform(ID id, vmath::mat4 matrix)
{
	if (!has(id)) { plog::error("Entity %s has no transform component!", toString(id)); return; }
	size_t index = map[id];  matrices[index] = matrix * matrices[index];
}

//// Other stuff ////

void Flip()
{
	// Should this be one big loop instead of 3 loops?
	// 3 loops might be more cache-friendly...   maybe.
	// Testing would be needed, but I really doubt this is a bottleneck.

	for (size_t i = 0; i < entries; ++i)
		{ prev_pos[i] = positions[i]; }

	for (size_t i = 0; i < entries; ++i)
		{ prev_rot[i] = rotations[i]; }

	for (size_t i = 0; i < entries; ++i)
		{ prev_scl[i] = scales[i]; }
}

void CalcMatrices(float interpolation)
{
	for (size_t i = 0; i < entries; ++i)
	{
		matrices[i] = mat4::translation(vec3::lerp(prev_pos[i], positions[i], interpolation)) *
					  mat4::rotation(quat::lerp(prev_rot[i], rotations[i], interpolation)) *
					  mat4::scale(vec3::lerp(prev_scl[i], scales[i], interpolation));
	}
}

}} // namespace entity::transform