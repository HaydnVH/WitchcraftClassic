#include "physical.h"

#include "physics/physics_system.h"
#include "physics/layers.h"
using namespace vmath;

#include "graphics/model.h"

void ColliderComponent::Delete(entity::ID id)
{
	if (hasEntry(id) == false) return;
	btCollisionShape* ptr = soa.get<PCE_SHAPEPTR>(index(id));
	if (ptr == nullptr) return;
	if (soa.get<PCE_COLLIDERTYPE>(index(id)) == COLLIDER_COMPOUND)
	{
		btCompoundShape* compound = (btCompoundShape*)ptr;
		for (int i = 0; i < compound->getNumChildShapes(); ++i)
		{
			btCollisionShape* shape = compound->getChildShape(i);
			compound->removeChildShapeByIndex(i);
			if (shape != nullptr)
				{ delete shape; }
		}
	}
	if (soa.get<PCE_MESHINTERFACE>(index(id)))
		delete soa.get<PCE_MESHINTERFACE>(index(id));

	delete ptr;
	RemoveEntry(id);
}

void ColliderComponent::MakeSimpleShape(entity::ID id, SimpleShapeCollider shape, vec3 offset_pos, quat offset_rot)
{
	if (hasEntry(id) == true)
	{
		plog::error("Attempting to attach a collider to entity which already has a collider.\n");
		return;
	}

	AddEntry(id, COLLIDER_NULL, nullptr, nullptr, VEC3_ZERO, QUAT_IDENTITY);

	btCollisionShape* collider = shape.makeCollider();

	soa.get<PCE_COLLIDERTYPE>(index(id)) = shape.type;
	soa.get<PCE_SHAPEPTR>(index(id)) = collider;
	soa.get<PCE_OFFSETPOS>(index(id)) = offset_pos;
	soa.get<PCE_OFFSETROT>(index(id)) = offset_rot;
}

void ColliderComponent::MakeCompoundShape(entity::ID id, StructOfArrays<SimpleShapeCollider, vmath::vec3, vmath::quat>& shapes)
{
	if (hasEntry(id) == true)
	{
		plog::error("Attempting to attach a collider to entity which already has a collider.\n");
		return;
	}

	AddEntry(id, COLLIDER_NULL, nullptr, nullptr, VEC3_ZERO, QUAT_IDENTITY);

	btCompoundShape* collider = new btCompoundShape(true, (int)shapes.size());

	for (size_t i = 0; i < shapes.size(); ++i)
	{
		SimpleShapeCollider ss = shapes.get<0>(i);
		btCollisionShape* shape = ss.makeCollider();

		vec3 pos = shapes.get<1>(i);
		quat rot = shapes.get<2>(i);

		btTransform transform({ rot.x, rot.y, rot.z, rot.w }, { pos.x, pos.y, pos.z });
		collider->addChildShape(transform, shape);
	}

	soa.get<PCE_COLLIDERTYPE>(index(id)) = COLLIDER_COMPOUND;
	soa.get<PCE_SHAPEPTR>(index(id)) = collider;
	soa.get<PCE_OFFSETPOS>(index(id)) = VEC3_ZERO;
	soa.get<PCE_OFFSETROT>(index(id)) = QUAT_IDENTITY;
}

void ColliderComponent::MakeColliderFromModel(entity::ID id)
{
	if (hasEntry(id) == true)
	{
		plog::error("Attempting to attach a collider to entity which already has a collider.\n");
		return;
	}

	Model* modelptr = rc.getModelPtr(id);
	if (modelptr == nullptr)
	{
		plog::error("Attempting to create a collider from a model when no model is present.\n");
		return;
	}

	switch (modelptr->getCollision().type)
	{
	case COLLIDER_SIMPLE:
		MakeSimpleShape(id, modelptr->getCollision().simple_collider_shapes[0], modelptr->getCollision().offset_position, modelptr->getCollision().offset_rotation);
		return;
	case COLLIDER_COMPOUND:
		// TODO: this
		break;
	case COLLIDER_CONVEXMESH:
		// TODO: this
		break;
	case COLLIDER_CONCAVEMESH:
		{
			AddEntry(id, COLLIDER_NULL, nullptr, nullptr, VEC3_ZERO, QUAT_IDENTITY);

			btStridingMeshInterface* mesh = new btTriangleIndexVertexArray(
				(int)modelptr->getCollision().num_indices / 3, modelptr->getCollision().indices_ptr, sizeof(int) * 3,
				(int)modelptr->getCollision().num_vertices, (float*)modelptr->getCollision().vertices_ptr, sizeof(VertexPosition));

			btCollisionShape* shape = new btBvhTriangleMeshShape(mesh, true, true);
			soa.get<PCE_COLLIDERTYPE>(index(id)) = COLLIDER_CONCAVEMESH;
			soa.get<PCE_SHAPEPTR>(index(id)) = shape;
			soa.get<PCE_MESHINTERFACE>(index(id)) = mesh;
			soa.get<PCE_OFFSETPOS>(index(id)) = VEC3_ZERO;
			soa.get<PCE_OFFSETROT>(index(id)) = QUAT_IDENTITY;
		}
		break;
	default:
		plog::error("Attached model does not have collision information, or has invalid collision information.\n");
		return;
	}
}

void RigidBodyComponent::Create(entity::ID id, float mass, bool kinematic, bool triggervolume, int group, int mask)
{
	if (hasEntry(id) == true)
	{
		plog::error("Cannot add a rigid body to an entity which already has a rigid body.\n");
		return;
	}

	AddEntry(id, nullptr, nullptr);

	if (cc.hasEntry(id) == false)
	{
		plog::error("Cannot add a rigid body to an entity which lacks a collider.\n");
		return;
	}

	btCollisionShape* collider = cc.soa.get<PCE_SHAPEPTR>(cc.index(id));
	if (collider == nullptr)
	{
		plog::error("Cannot add a rigid body to an entity which lacks a collider.\n");
		return;
	}

	if (soa.get<PCE_RIGIDBODYPTR>(index(id)) != nullptr)
	{
		plog::error("This entity already has a rigid body.\n");
		return;
	}

	// Get the collider's offset (should be the identity in the case of a non-simple shape).
	quat rot = cc.soa.get<PCE_OFFSETROT>(cc.index(id));
	vec3 pos = cc.soa.get<PCE_OFFSETPOS>(cc.index(id));

	// Get the object's world transform and add the collider's offset.
	pos += entity::transform::getPos(id);
	rot *= entity::transform::getRot(id); // should this be "rot = tc.getRotation(id) * rot"? Testing needed.

	// Create the object's initial transform using the position and rotation passed in.
	btTransform start_transform({ rot.x, rot.y, rot.z, rot.w }, { pos.x, pos.y, pos.z });

	btVector3 local_inertia(0, 0, 0);
	if (mass > 0)
	{
		if (cc.soa.get<PCE_COLLIDERTYPE>(cc.index(id)) == COLLIDER_CONCAVEMESH)
		{
			plog::error("Concave mesh colliders cannot be used for dynamic rigidbodies.\n");
			return;
		}

		if (triggervolume)
		{
			plog::error("Dynamic rigidbodies cannot be trigger volumes.\n");
			return;
		}


		collider->calculateLocalInertia(mass, local_inertia);
	}

	btMotionState* motionstate = new btDefaultMotionState(start_transform);
	btRigidBody::btRigidBodyConstructionInfo rbinfo(mass, motionstate, collider, local_inertia);

	btRigidBody* rigidbody = new btRigidBody(rbinfo);

	rigidbody->setUserIndex(id);
	rigidbody->setUserIndex2(0);

	if (kinematic)
	{
		rigidbody->setCollisionFlags(rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidbody->setActivationState(DISABLE_DEACTIVATION);
	}

	if (triggervolume)
	{
		rigidbody->setCollisionFlags(rigidbody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}

	physics::getWorld().addRigidBody(rigidbody, group, mask);

	soa.get<PCE_RIGIDBODYPTR>(index(id)) = rigidbody;
	soa.get<PCE_MOTIONSTATEPTR>(index(id)) = motionstate;

	if (mass > 0)
		dynamic_entities.insert(id);

	if (kinematic)
		kinematic_entities.insert(id);
}

void RigidBodyComponent::Delete(entity::ID id)
{
	if (hasEntry(id) == false) return;
	btRigidBody* rb = soa.get<PCE_RIGIDBODYPTR>(index(id));
	if (rb != nullptr)
	{
		physics::getWorld().removeRigidBody(rb);
		delete rb;
	}
	btMotionState* ms = soa.get<PCE_MOTIONSTATEPTR>(index(id));
	if (ms != nullptr) delete ms;
	dynamic_entities.erase(id);
	RemoveEntry(id);
}

std::vector<OverlapResult> RigidBodyComponent::OverlapTestStationary(entity::ID id, int mask)
{
	if (hasEntry(id) == false) return {};
	btRigidBody* rb = soa.get<PCE_RIGIDBODYPTR>(index(id));
	if (rb == nullptr) return {};

	OverlapList callback;
	callback.m_collisionFilterGroup = PHYSICSLAYER_DEFAULT;
	callback.m_collisionFilterMask = mask;

	physics::getWorld().contactTest(rb, callback);

	return callback.hits;
}

std::vector<OverlapResult> RigidBodyComponent::OverlapTestContinuous(entity::ID id, int mask)
{
	if (hasEntry(id) == false) return {};
	btRigidBody* rb = soa.get<PCE_RIGIDBODYPTR>(index(id));
	if (rb == nullptr) return {};

	if (rb->getCollisionShape()->isConvex() == false)
	{
		plog::error("Cannot use OverlapTestContinuous on a collider which is not convex.\n");
		return {};
	}

	// Maybe a dangerous cast?
	btConvexShape* collider = (btConvexShape*)rb->getCollisionShape();

	vec3 prev_position = entity::transform::interpolatePos(id, 0);
	quat prev_rotation = entity::transform::interpolateRot(id, 0);
	btTransform prev_transform({ prev_rotation.x, prev_rotation.y, prev_rotation.z, prev_rotation.w },
								{ prev_position.x, prev_position.y, prev_position.z });

	vec3 now_position = entity::transform::getPos(id);
	quat now_rotation = entity::transform::getRot(id);
	btTransform now_transform({ now_rotation.x, now_rotation.y, now_rotation.z, now_rotation.w },
								{ now_position.x, now_position.y, now_position.z });

	OverlapSweepList callback;
	callback.m_collisionFilterGroup = PHYSICSLAYER_DEFAULT;
	callback.m_collisionFilterMask = mask;

	physics::getWorld().convexSweepTest(collider, prev_transform, now_transform, callback);

	// Sort the entries in the list of hits such that nearest hits are first.
	// This might not be neccesary, depending on the order Bullet adds entries.
	std::sort(callback.hits.begin(), callback.hits.end(), [](const OverlapResult& lhs, const OverlapResult& rhs)
		{ return (lhs.distance < rhs.distance); });

	return callback.hits;
}

void RigidBodyComponent::ApplySimulatedTransforms()
{
	for (auto& id : dynamic_entities)
	{
		btMotionState* motionstate = soa.get<PCE_MOTIONSTATEPTR>(index(id) );
		if (motionstate == nullptr) continue;

		btTransform transform;
		motionstate->getWorldTransform(transform);

		btQuaternion rot = transform.getRotation();
		btVector3 pos = transform.getOrigin();

		// Do we have to take the collider's offset into account here?
		// Probably, but testing is needed!

		entity::transform::setPos(id, vec3(pos.x(), pos.y(), pos.z()) - cc.soa.get<PCE_OFFSETPOS>(cc.index(id)) );
		entity::transform::setRot(id, quat(rot.x(), rot.y(), rot.z(), rot.w()) );
	}
}

void RigidBodyComponent::ApplyKinematicTransforms()
{
	for (auto& id : kinematic_entities)
	{
		btMotionState* motionstate = soa.get<PCE_MOTIONSTATEPTR>(index(id));
		if (motionstate == nullptr) continue;

		vec3 pos = entity::transform::getPos(id) + cc.soa.get<PCE_OFFSETPOS>(cc.index(id) );
		quat rot = entity::transform::getRot(id);

		btTransform transform({rot.x, rot.y, rot.z, rot.w}, {pos.x, pos.y, pos.z});
		motionstate->setWorldTransform(transform);
	}
}