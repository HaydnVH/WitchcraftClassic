#include "collider.h"

#include <btBulletCollisionCommon.h>
#include "sys/printlog.h"

const std::map<std::string, ColliderTypeEnum> COLLIDER_TYPE_STR_TO_ENUM =
{
	{"null",	COLLIDER_NULL},
	{"sphere",	COLLIDER_SPHERE},
	{"box",		COLLIDER_BOX},
	{"cylinder",COLLIDER_CYLINDER},
	{"capsule", COLLIDER_CAPSULE},
	{"cone",	COLLIDER_CONE},
	{"cylinder_x", COLLIDER_CYLINDER_X},
	{"cylinder_y", COLLIDER_CYLINDER_Y},
	{"cylinder_z", COLLIDER_CYLINDER},
	{"capsule_x", COLLIDER_CAPSULE_X},
	{"capsule_y", COLLIDER_CAPSULE_Y},
	{"capsule_z", COLLIDER_CAPSULE},
	{"cone_x", COLLIDER_CONE_X},
	{"cone_y", COLLIDER_CONE_Y},
	{"cone_z", COLLIDER_CONE},
	{"simple",	COLLIDER_SIMPLE},
	{"compound",COLLIDER_COMPOUND},
	{"convex mesh",	COLLIDER_CONVEXMESH},
	{"concave mesh",COLLIDER_CONCAVEMESH}
};


const char* ColliderTypeEnumToStr(ColliderTypeEnum type)
{
	switch (type)
	{
		case COLLIDER_NULL:			return "null";
		case COLLIDER_SPHERE:		return "sphere" ;
		case COLLIDER_BOX:			return "box" ;
		case COLLIDER_CYLINDER:		return "cylinder" ;
		case COLLIDER_CAPSULE:		return "capsule" ;
		case COLLIDER_CONE:			return "cone" ;
		case COLLIDER_CYLINDER_X:	return "cylinder_x";
		case COLLIDER_CYLINDER_Y:	return "cylinder_y";
		case COLLIDER_CAPSULE_X:	return "capsule_x";
		case COLLIDER_CAPSULE_Y:	return "capsule_y";
		case COLLIDER_CONE_X:		return "cone_x";
		case COLLIDER_CONE_Y:		return "cone_y";
		case COLLIDER_SIMPLE:		return "simple" ;
		case COLLIDER_COMPOUND:		return "compound" ;
		case COLLIDER_CONVEXMESH:	return "convex mesh" ;
		case COLLIDER_CONCAVEMESH:	return "concave mesh" ;
		default: return "invalid";
	}
}

btConvexShape* SimpleShapeCollider::makeCollider()
{
	switch (type)
	{
	case COLLIDER_SPHERE:
		return new btSphereShape(args.x);
		break;
	case COLLIDER_BOX:
		return new btBoxShape({ args.x, args.y, args.z });
		break;
	case COLLIDER_CYLINDER:
		return new btCylinderShapeZ({ args.x, args.y, args.z });
		break;
	case COLLIDER_CAPSULE:
		return new btCapsuleShapeZ(args.x, args.y);
		break;
	case COLLIDER_CONE:
		return new btConeShapeZ(args.x, args.y);
		break;
	case COLLIDER_CYLINDER_X:
		return new btCylinderShapeX({ args.x, args.y, args.z });
		break;
	case COLLIDER_CYLINDER_Y:
		return new btCylinderShape({ args.x, args.y, args.z });
		break;
	case COLLIDER_CAPSULE_X:
		return new btCapsuleShapeX(args.x, args.y);
		break;
	case COLLIDER_CAPSULE_Y:
		return new btCapsuleShape(args.x, args.y);
		break;
	case COLLIDER_CONE_X:
		return new btConeShapeX(args.x, args.x);
		break;
	case COLLIDER_CONE_Y:
		return new btConeShape(args.x, args.y);
		break;
	default:
		plog::error("Invalid simple shape enum.\n");
		return nullptr;
	}
}