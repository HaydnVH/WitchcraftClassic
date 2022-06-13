#include "animator.h"

#include "graphics/model.h"
#include "physics//physics_system.h"

void AnimatorComponent::ImportAnimations(entity::ID id, const char* filename)
{
	Model* modelptr = rc.getModelPtr(id);
	if (modelptr) modelptr->ImportAnimations(filename);
}

void AnimatorComponent::LogicalUpdate(double delta_time)
{
	for (size_t i = 1; i < soa.size(); ++i)
	{
		entity::ID id = soa.get<ACE_ENTITYID>(i);

		vmath::mat4 transform = vmath::mat4::translation(entity::transform::getPos(id)) *
								vmath::mat4::rotation(entity::transform::getRot(id)) *
								vmath::mat4::scale(entity::transform::getScale(id));

		std::vector<const char*> events = soa.get<ACE_ANIMCONTROLLER>(i).CalcNextFrame(transform, delta_time);
	}
	// TODO: Actually handle animation events!
}

void AnimatorComponent::getAnimatedTransform(entity::ID id, const char* bone_name, vmath::vec3& out_position, vmath::quat& out_rotation)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	vmath::mat4 transform = vmath::mat4::translation(entity::transform::getPos(id)) *
							vmath::mat4::rotation(entity::transform::getRot(id)) *
							vmath::mat4::scale(entity::transform::getScale(id));

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimatedBoneTransform(bone_index, out_position, out_rotation, transform);
}

void AnimatorComponent::setBoneAdditivePosition(entity::ID id, const char* bone_name, vmath::vec3 position)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	soa.get<ACE_ANIMCONTROLLER>(index(id)).setBoneAdditivePosition(bone_index, position);
}

void AnimatorComponent::setBoneAdditivePositionLocal(entity::ID id, const char* bone_name, vmath::vec3 position)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	soa.get<ACE_ANIMCONTROLLER>(index(id)).setBoneAdditivePositionLocal(bone_index, position);
}

vmath::vec3 AnimatorComponent::getBoneAdditivePosition(entity::ID id, const char* bone_name)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return vmath::VEC3_ZERO;

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	return soa.get<ACE_ANIMCONTROLLER>(index(id)).getBoneAdditivePosition(bone_index);
}

void AnimatorComponent::ClearAdditivePositions(entity::ID id)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	soa.get<ACE_ANIMCONTROLLER>(index(id)).ClearAdditivePositions();
}


void AnimatorComponent::setBoneAdditiveRotation(entity::ID id, const char* bone_name, vmath::quat rotation)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	soa.get<ACE_ANIMCONTROLLER>(index(id)).setBoneAdditiveRotation(bone_index, rotation);
}

void AnimatorComponent::setBoneAdditiveRotationLocal(entity::ID id, const char* bone_name, vmath::quat rotation)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	soa.get<ACE_ANIMCONTROLLER>(index(id)).setBoneAdditiveRotationLocal(bone_index, rotation);
}

vmath::quat AnimatorComponent::getBoneAdditiveRotation(entity::ID id, const char* bone_name)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return vmath::QUAT_IDENTITY;

	int bone_index = rc.getModelPtr(id)->getSkeleton().bone_map.at(bone_name);
	return soa.get<ACE_ANIMCONTROLLER>(index(id)).getBoneAdditiveRotation(bone_index);
}

void AnimatorComponent::ClearAdditiveRotations(entity::ID id)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	soa.get<ACE_ANIMCONTROLLER>(index(id)).ClearAdditiveRotations();
}

void AnimatorComponent::DrawDebug(entity::ID id)
{
	if (hasEntry(id) == false || rc.hasEntry(id) == false)
		return;

	vmath::mat4 transform = vmath::mat4::translation(entity::transform::getPos(id)) *
							vmath::mat4::rotation(entity::transform::getRot(id)) *
							vmath::mat4::scale(entity::transform::getScale(id));

	Model* model = rc.getModelPtr(id);

	for (int bone_index = 0; bone_index < model->getSkeleton().num_bones; ++bone_index)
	{
		int parent_index = model->getSkeleton().parent_index[bone_index];
		if (parent_index < 0 || parent_index >= model->getSkeleton().num_bones)
			continue;

		vmath::vec3 my_position;
		vmath::quat my_rotation;
		soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimatedBoneTransform(bone_index, my_position, my_rotation, transform);
		vmath::vec3 parent_position;
		vmath::quat parent_rotation;
		soa.get<ACE_ANIMCONTROLLER>(index(id)).getAnimatedBoneTransform(parent_index, parent_position, parent_rotation, transform);

		physics::getDebug().drawLine({my_position.x, my_position.y, my_position.z}, {parent_position.x, parent_position.y, parent_position.z}, {0.3f, 0.8f, 0.8f});
	}
}