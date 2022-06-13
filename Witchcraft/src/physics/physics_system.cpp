#include "physics_system.h"

#include "sys/printlog.h"

using namespace vmath;

namespace physics {

namespace {

	btDefaultCollisionConfiguration collision_config;
	btCollisionDispatcher dispatcher(&collision_config);
	btDbvtBroadphase broadphase;
	btSequentialImpulseConstraintSolver solver;
	btDiscreteDynamicsWorld world(&dispatcher, &broadphase, &solver, &collision_config);
	PhysicsDebug debug;

}

bool Init()
{
	static bool initialized = false;
	if (initialized)
	{
		plog::fatal("Programmer Error: Attempted to intialize physics system more than once!");
		return false;
	}
	initialized = true;

	setGravity(vmath::VEC3_DOWN * 9.81f * 2.0f);

	// This object's initialization calls OpenGL code, making this whole subsystem depend on Window.
	debug.Init();
	world.setDebugDrawer(&debug);

	InitLua();

	return true;
}

void LogicalUpdate(float delta_time)
{
	world.stepSimulation(delta_time, 4);
	world.debugDrawWorld();
}

void setGravity(vmath::vec3 gravity)
	{ world.setGravity(btVector3(gravity.x, gravity.y, gravity.z)); }

PhysicsDebug& getDebug()
	{ return debug; }

btDiscreteDynamicsWorld& getWorld()
	{ return world; }

RaycastResult RaycastNearest(vec3 from, vec3 to, int layermask)
{
	RaycastResult result = {};

	btCollisionWorld::ClosestRayResultCallback callback({ from.x, from.y, from.z }, {to.x, to.y, to.z});
	callback.m_collisionFilterGroup = 1; // PHYSICSLAYER_DEFAULT
	callback.m_collisionFilterMask = layermask;

	world.rayTest({ from.x, from.y, from.z }, { to.x, to.y, to.z }, callback);
	result.hit = callback.hasHit();
	if (result.hit)
	{
		result.normal = vec3(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z()).normalized();
		result.position = vec3(callback.m_hitPointWorld.x(), callback.m_hitPointWorld.y(), callback.m_hitPointWorld.z());
		result.sweep_dist = vec3::distance(from, result.position);
		result.id = callback.m_collisionObject->getUserIndex();
	}

	return result;
}

RaycastResult ShapecastNearest(vec3 from, vec3 to, SimpleShapeCollider shape, int layermask)
{
	RaycastResult result = {};

	btConvexShape* collider = shape.makeCollider();
	btCollisionWorld::ClosestConvexResultCallback callback({from.x, from.y, from.z}, {to.x, to.y, to.z});
	callback.m_collisionFilterGroup = 1; // PHYSICSLAYER_DEFAULT
	callback.m_collisionFilterMask = layermask;

	world.convexSweepTest(collider, btTransform({ 0,0,0,1 }, { from.x,from.y,from.z }), btTransform({ 0,0,0,1 }, { to.x,to.y,to.z }), callback);
	result.hit = callback.hasHit();
	if (result.hit)
	{
		result.normal = vec3(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z()).normalized();
		result.position = vec3(callback.m_hitPointWorld.x(), callback.m_hitPointWorld.y(), callback.m_hitPointWorld.z());
		result.sweep_dist = vec3::distance(from, to) * callback.m_closestHitFraction;
		result.id = callback.m_hitCollisionObject->getUserIndex();
	}

	delete collider;
	return result;
}

} // namespace physics