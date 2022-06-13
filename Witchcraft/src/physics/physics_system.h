#ifndef HVH_WC_PHYSICS_PHYSICSSYSTEM_H
#define HVH_WC_PHYSICS_PHYSICSSYSTEM_H

#include <btBulletDynamicsCommon.h>
#include "math/vmath.h"

#include "collider.h"
#include "debug_physics.h"
#include "scene/entity.h"


struct RaycastResult
{
	bool hit;
	vmath::vec3 position;
	vmath::vec3 normal;
	float sweep_dist;
	entity::ID id;
};

namespace physics
{
	/* Sets up the physics environment. */
	/* Must initialize after: Window. */
	/* Returns false if initialization fails. */
	bool Init();

	/* Updates the physical world. */
	void LogicalUpdate(float delta_time);

	/* Sets the global gravitational constant, for which all things shall obey. */
	void setGravity(vmath::vec3 gravity);

	/* Cast a ray from one point to another, detecting any collisions along the way. */
	RaycastResult RaycastNearest(vmath::vec3 from, vmath::vec3 to, int layermask = -1);
	/* Cast a solid shape from one point to another, detecting any collisions along the way. */
	RaycastResult ShapecastNearest(vmath::vec3 from, vmath::vec3 to, SimpleShapeCollider shape, int layermask = -1);

	/* Get a reference to the object which handles Bullet's debug drawing. */
	PhysicsDebug& getDebug();
	/* Get a reference to the physics system's world.  Handle with care! */
	btDiscreteDynamicsWorld& getWorld();

	/* Initializes the Lua interface for the physics system. */
	/* This is called automatically by Init(), and should never be called by outside code. */
	void InitLua();
};

#endif