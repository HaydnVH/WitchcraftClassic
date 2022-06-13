#ifndef HVH_WC_PHYSICS_OVERLAPLIST_H
#define HVH_WC_PHYSICS_OVERLAPLIST_H

#include <btBulletDynamicsCommon.h>
#include <vector>
#include "scene/entity.h"

struct OverlapResult
{
	entity::ID id;
	int bodypart;
	float distance;
};

struct OverlapList : public btCollisionWorld::ContactResultCallback
{
	float addSingleResult(btManifoldPoint &cp,
		const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
		const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1)
	{
		OverlapResult result = {};
		if (colObj0Wrap->getCollisionObject() == me)
		{
			result.id = colObj1Wrap->getCollisionObject()->getUserIndex();
			result.bodypart = colObj1Wrap->getCollisionObject()->getUserIndex2();
		}
		else
		{
			result.id = colObj0Wrap->getCollisionObject()->getUserIndex();
			result.bodypart = colObj0Wrap->getCollisionObject()->getUserIndex2();
		}
		hits.push_back(result);
		return 0.0f;
	}

	btCollisionObject* me;

	std::vector<OverlapResult> hits;
};

struct OverlapSweepList : public btCollisionWorld::ConvexResultCallback
{
	float addSingleResult(btCollisionWorld::LocalConvexResult &convexResult, bool normalInWorldSpace)
	{
		OverlapResult result = {};
		result.id = convexResult.m_hitCollisionObject->getUserIndex();
		result.bodypart = convexResult.m_hitCollisionObject->getUserIndex2();
		result.distance = convexResult.m_hitFraction;
		hits.push_back(result);
		return 0.0f;
	}

	std::vector<OverlapResult> hits;
};

#endif // HVH_WC_PHYSICS_OVERLAPLIST_H