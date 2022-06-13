#ifndef HVH_WC_PHYSICS_COLLIDER_H
#define HVH_WC_PHYSICS_COLLIDER_H

#include "math/vmath.h"
#include <map>
#include <string>

enum ColliderTypeEnum
{
	COLLIDER_NULL = 0,
	COLLIDER_SPHERE,
	COLLIDER_BOX,
	COLLIDER_CYLINDER,
	COLLIDER_CAPSULE,
	COLLIDER_CONE,
	COLLIDER_CYLINDER_X,
	COLLIDER_CYLINDER_Y,
	COLLIDER_CAPSULE_X,
	COLLIDER_CAPSULE_Y,
	COLLIDER_CONE_X,
	COLLIDER_CONE_Y,
	COLLIDER_SIMPLE,
	COLLIDER_COMPOUND,
	COLLIDER_CONVEXMESH,
	COLLIDER_CONCAVEMESH
};

extern const std::map<std::string, ColliderTypeEnum> COLLIDER_TYPE_STR_TO_ENUM;
const char* ColliderTypeEnumToStr(ColliderTypeEnum);

class btConvexShape;

struct SimpleShapeCollider
{
	SimpleShapeCollider()
		: type(COLLIDER_NULL), args(vmath::VEC3_ZERO) {}

	SimpleShapeCollider(ColliderTypeEnum type, float sphere_radius)
	: type(type), args(sphere_radius, 0.0f, 0.0f) {}

	SimpleShapeCollider(ColliderTypeEnum type, vmath::vec3 half_extends)
	: type(type), args(half_extends) {}

	SimpleShapeCollider(ColliderTypeEnum type, float radius, float height)
	: type(type), args(radius, height, 0.0f) {}

	ColliderTypeEnum type;
	vmath::vec3 args;

	btConvexShape* makeCollider();
};
 

#endif // HVH_WC_PHYSICS_COLLIDER_H