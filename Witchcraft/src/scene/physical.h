#ifndef HVH_WC_SCENE_PHYSICALCOMPONENT_H
#define HVH_WC_SCENE_PHYSICALCOMPONENT_H

#include <set>

#include "component.h"
#include "transform.h"
#include "renderable.h"

#include "math/vmath.h"
#include <btBulletDynamicsCommon.h>

#include "physics/collider.h"
#include "physics/overlap_list.h"

/* 5 different kinds of pysical objects need to be presented:
- Dynamic RigidBody
- Kinematic RigidBody
- Static RigidBody
- Kinematic Ghost
- Static Ghost

Dynamic Rigidbody is the only one that we actually need to iterate over, to get the simulated transform.
The rest we simply need to keep around so we can access.

Should colliders exist as their own component?
They are common to each of these, but they don't make any sense by themselves.
Then again, neither does Transform...
*/

enum PhysicalComponentEnum
{
	PCE_ENTITYID = 0,
	PCE_COLLIDERTYPE,
	PCE_SHAPEPTR,
	PCE_MESHINTERFACE,
	PCE_OFFSETPOS,
	PCE_OFFSETROT,
	PCE_RIGIDBODYPTR = 1,
	PCE_MOTIONSTATEPTR,
	PCE_GHOSTOBJECTPTR = 1
};

class ColliderComponent : protected ComponentTable<ColliderTypeEnum, btCollisionShape*, btStridingMeshInterface*, vmath::vec3, vmath::quat>
{
	friend class RigidBodyComponent;

public:
	ColliderComponent(RenderableComponent& rc)
	:	ComponentTable<ColliderTypeEnum, btCollisionShape*, btStridingMeshInterface*, vmath::vec3, vmath::quat>(COLLIDER_NULL, nullptr, nullptr, vmath::VEC3_ZERO, vmath::QUAT_IDENTITY),
		rc(rc)
		{}

	void Delete(entity::ID id);

	bool isValid(entity::ID id)
	{
		if (hasEntry(id) == false) return false;
		if (soa.get<PCE_SHAPEPTR>(index(id)) == nullptr) return false;
		return true;
	}

	void MakeSimpleShape(entity::ID id, SimpleShapeCollider shape, vmath::vec3 pos_offset = vmath::VEC3_ZERO, vmath::quat rot_offset = vmath::QUAT_IDENTITY);
	void MakeCompoundShape(entity::ID id, StructOfArrays<SimpleShapeCollider, vmath::vec3, vmath::quat>& shapes);
	void MakeColliderFromModel(entity::ID id);

	void setPosOffset(entity::ID id, vmath::vec3 new_offset)
		{ if (hasEntry(id)) { soa.get<PCE_OFFSETPOS>(index(id)) = new_offset; } }

	void setRotOffset(entity::ID id, vmath::quat new_offset)
		{ if (hasEntry(id)) { soa.get<PCE_OFFSETROT>(index(id)) = new_offset; } }

private:
	RenderableComponent& rc;
};

class RigidBodyComponent : protected ComponentTable<btRigidBody*, btMotionState*>
{
public:
	RigidBodyComponent(ColliderComponent& cc)
	:	ComponentTable<btRigidBody*, btMotionState*>(nullptr, nullptr),
		cc(cc)
		{}

	void Delete(entity::ID id);

	bool isValid(entity::ID id)
	{
		if (hasEntry(id) == false) return false;
		if (soa.get<PCE_RIGIDBODYPTR>(index(id)) == nullptr) return false;
		return true;
	}

	void Create(entity::ID id, float mass, bool kinematic, bool triggervolume, int group, int mask);

	// Performs an in-place contact test and returns a list of entities which are currently overlapping.
	std::vector<OverlapResult> OverlapTestStationary(entity::ID id, int mask = -1);

	// Sweeps the object's convex collider between its previous frame and its current frame,
	// returning a list of entities which overlap at any point along the sweep.
	std::vector<OverlapResult> OverlapTestContinuous(entity::ID id, int mask = -1);

	void ApplySimulatedTransforms();
	void ApplyKinematicTransforms();

private:
	ColliderComponent& cc;

	std::set<entity::ID> dynamic_entities;
	std::set<entity::ID> kinematic_entities;
};

void PhysicalInitLua(ColliderComponent&, RigidBodyComponent&);

#endif // HVH_WC_SCENE_PHYSICALCOMPONENT_H