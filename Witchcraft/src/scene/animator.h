#ifndef HVH_WC_SCENE_ANIMATOR_H
#define HVH_WC_SCENE_ANIMATOR_H

#include "component.h"
#include "renderable.h"
#include "graphics/animation_controller.h"

class PhysicsSystem;

enum AnimatorComponentEnum
{
	ACE_ENTITYID = 0,
	ACE_ANIMCONTROLLER
};

class AnimatorComponent : protected ComponentTable<AnimationController>
{
public:
	AnimatorComponent(RenderableComponent& rc)
	:	ComponentTable<AnimationController>({ nullptr, {} }),
		rc(rc), ps(ps)
		{}
	~AnimatorComponent() {}

	using ComponentTable<AnimationController>::hasEntry;

	void AddTo(entity::ID id)
	{
		if (rc.hasEntry(id) == false)
		{
			plog::error("Cannot add animator component to an entity which lacks a renderable component.\n");
			return;
		}

		Model* modelptr = rc.getModelPtr(id);
		if (modelptr == nullptr)
		{
			plog::error("Load a model using this entity's renderable component before attaching an animator component.\n");
			return;
		}

		AddEntry(id, {modelptr, id});
	}

	using ComponentTable<AnimationController>::RemoveEntry;

	void ImportAnimations(entity::ID id, const char* filename);

	void PlayAnimation(entity::ID id, int layer, const char* animname, bool loop = false, float speed = 1.0f, float transition = 0.0f)
		{ if (rc.hasEntry(id)) soa.get<ACE_ANIMCONTROLLER>(index(id)).PlayAnimation(layer, animname, loop, speed, transition); }

	const char* getCurrentAnimName(entity::ID id, int layer)
		{ if (rc.hasEntry(id)) return soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimName(layer); else return ""; }

	float getAnimSpeed(entity::ID id)
		{ return soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimSpeed(); }
	void setAnimSpeed(entity::ID id, float val)
		{ if (hasEntry(id)) soa.get<ACE_ANIMCONTROLLER>(index(id)).setAnimSpeed(val); }

	bool isLooping(entity::ID id, int layer)
		{ return soa.get<ACE_ANIMCONTROLLER>(index(id)).isLooping(layer); }
	void setLooping(entity::ID id, int layer, bool val)
		{ if (hasEntry(id)) soa.get<ACE_ANIMCONTROLLER>(index(id)).setLooping(layer, val); }

	float getAnimDuration(entity::ID id, int layer)
		{ if (hasEntry(id)) return soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimDuration(layer); else return 0.0f; }
	float getAnimTime(entity::ID id, int layer)
		{ return soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimTime(layer); }

	void setAnimLayerWeight(entity::ID id, int layer, float val)
		{ if (hasEntry(id)) return soa.get<ACE_ANIMCONTROLLER>(index(id)).setAnimLayerWeight(layer, val); }
	float getAnimLayerWeight(entity::ID id, int layer)
		{ return soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimLayerWeight(layer); }

	void getAnimatedTransform(entity::ID id, const char* bone_name, vmath::vec3& out_position, vmath::quat& out_rotation);

	void setBoneAdditivePosition(entity::ID id, const char* bone_name, vmath::vec3 position);
	void setBoneAdditivePositionLocal(entity::ID id, const char* bone_name, vmath::vec3 position);
	vmath::vec3 getBoneAdditivePosition(entity::ID id, const char* bone_name);
	void ClearAdditivePositions(entity::ID id);

	void setBoneAdditiveRotation(entity::ID id, const char* bone_name, vmath::quat rotation);
	void setBoneAdditiveRotationLocal(entity::ID id, const char* bone_name, vmath::quat rotation);
	vmath::quat getBoneAdditiveRotation(entity::ID id, const char* bone_name);
	void ClearAdditiveRotations(entity::ID id);

	void DrawDebug(entity::ID id);


	void Flip()
	{
		for (size_t i = 1; i < soa.size(); ++i)
			{ soa.get<ACE_ANIMCONTROLLER>(i).Flip(); }
	}

	void LogicalUpdate(double delta_time = (1.0 / 30.0));

	void ApplyRagdolls();

	void DisplayUpdate(float interpolation)
	{
		for (size_t i = 1; i < soa.size(); ++i)
			{ soa.get<ACE_ANIMCONTROLLER>(i).CalcMatrices(interpolation); }
	}

	void UploadSkeleton(entity::ID id)
		{ soa.get<ACE_ANIMCONTROLLER>(index(id)).UploadMatrices(); }

	void InitLua();

private:
	RenderableComponent& rc;
	PhysicsSystem& ps;
};

#endif // HVH_WC_SCENE_ANIMATOR_H