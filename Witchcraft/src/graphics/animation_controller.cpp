#include "animation_controller.h"

#include "shader.h"
#include "model.h"
	
#include <btBulletDynamicsCommon.h>
#include "physics/physics_system.h"
#include "physics/layers.h"

#include "sys/printlog.h"

using namespace std;
using namespace vmath;

namespace
{
	uint32_t skeleton_ubo_index = 0;

} // namespace <anon>

AnimationController::AnimationController(Model* model, entity::ID id)
:	model(model),
	animspeed(1.0f)
{
	if (model)
	{
		bonedata.resize(model->getSkeleton().num_bones);

		for (size_t i = 0; i < bonedata.size(); ++i)
		{
			bonedata.get<ACM_PREV>(i) = MAT4_IDENTITY;
			bonedata.get<ACM_NOW>(i) = MAT4_IDENTITY;
			bonedata.get<ACM_FINAL>(i) = MAT4_IDENTITY;
			bonedata.get<AC_ADDPOS>(i) = VEC3_ZERO;
			bonedata.get<AC_ADDROT>(i) = QUAT_IDENTITY;
			bonedata.get<AC_RIGIDBODY>(i) = nullptr;
			bonedata.get<AC_MOTIONSTATE>(i) = nullptr;

			btCollisionShape* collider = model->getSkeleton().collider[i];
			if (collider != nullptr)
			{
				// Create a rigidbody for this bone, constrained to its parent.
				btTransform start_transform; start_transform.setIdentity();
				vec3 offset = model->getSkeleton().collider_offset[i];
				start_transform.setOrigin({ offset.x, offset.y, offset.z });

				btVector3 local_inertia(0, 0, 0);

				std::shared_ptr<btMotionState> motionstate(new btDefaultMotionState(start_transform));
				btRigidBody::btRigidBodyConstructionInfo rbinfo(0.0f, motionstate.get(), collider, local_inertia);

				std::shared_ptr<btRigidBody> rigidbody(new btRigidBody(rbinfo));

				// Set the entity ID and rigidbody bone ID here.
				rigidbody->setUserIndex(id);
				rigidbody->setUserIndex2(0);

				rigidbody->setCollisionFlags(rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
				rigidbody->setActivationState(DISABLE_DEACTIVATION);

				physics::getWorld().addRigidBody(rigidbody.get(), PHYSICSLAYER_PLAYERHITBOX, PHYSICSLAYER_DEFAULT | PHYSICSLAYER_ATTACK);

				bonedata.get<AC_RIGIDBODY>(i) = rigidbody;
				bonedata.get<AC_MOTIONSTATE>(i) = motionstate;
			}
		}

		for (int i = 0; i < NUM_ANIM_LAYERS; ++i)
		{
			layers[i].setmodel(model);
		}
	}

	layers[0].weight = 1.0f;
}

AnimationController::~AnimationController()
{
	for (size_t i = 0; i < bonedata.size(); ++i)
	{
		if (bonedata.get<AC_RIGIDBODY>(i) != nullptr)
		{
			if (bonedata.get<AC_RIGIDBODY>(i).use_count() == 1)
				{ physics::getWorld().removeRigidBody(bonedata.get<AC_RIGIDBODY>(i).get()); }
			bonedata.get<AC_RIGIDBODY>(i).reset();
		}

		if (bonedata.get<AC_MOTIONSTATE>(i) != nullptr)
			bonedata.get<AC_MOTIONSTATE>(i).reset();
	}
}

//int AnimationController::findAnimIndex(const char* anim_name)
//{
//	if (model)
//	{
//		return model->findAnimIndex(anim_name);
//	}
//	else
//	{
//		return -1;
//	}
//}

const char* AnimationController::getAnimName(int layer)
{
	if (layer < 0 || layer >= NUM_ANIM_LAYERS || model == nullptr)
		{ return "N/A"; }
	else
	{
		AnimationClip* clip = model->getAnimClip(layers[layer].now_state.anim_name.c_str);
		if (clip == nullptr)
			return "N/A";
		else
			return clip->name.c_str;
	}
}

float AnimationController::getAnimDuration(int layer)
{
	if (layer < 0 || layer >= NUM_ANIM_LAYERS || model == nullptr)
		{ return 0.0f; }
	else
	{
		AnimationClip* clip = model->getAnimClip(layers[layer].now_state.anim_name.c_str);
		if (clip == nullptr)
			return 0.0f;
		else
			return clip->duration;
	}
}

void AnimationController::initShaders()
{
	skeleton_ubo_index = Shader::RegisterUniformBuffer("SKELETON");
}

void AnimationController::Flip()
{
	memcpy(bonedata.rawdata<ACM_PREV>(), bonedata.rawdata<ACM_NOW>(), sizeof(mat4) * bonedata.size());
}

vector<const char*> AnimationController::AnimationState::AdvanceTime(float delta_time)
{
	if (model == nullptr) return {};
	AnimationClip* clip = model->getAnimClip(anim_name.c_str);
	if (clip == nullptr) return {};

	prev_time = now_time;
	now_time += delta_time * speed;

	if (looping)
	{
		while (now_time > clip->duration)
			{ now_time -= clip->duration; }
	}
	else if (now_time > clip->duration)
		{ now_time = clip->duration; }

	// Get any animation events which have occured, so scripts can respond to them.
	vector<const char*> result = clip->getEventsBetweenFrames(prev_time, now_time);

	return result;
}

void AnimationController::AnimationState::getTransformForBone(size_t bone_index, vec3& translation, quat& rotation, vec3& scale)
{
	if (model == nullptr) return;
	AnimationClip* clip = model->getAnimClip(anim_name.c_str);
	if (clip == nullptr) return;

	AnimationChannel& channel = clip->channels[bone_index];
	if (channel.isEmpty() == false)
	{
		translation = channel.getValueAtTime(channel.position_keys, now_time, looping, VEC3_ZERO);
		rotation = channel.getValueAtTime(channel.rotation_keys, now_time, looping, QUAT_IDENTITY);
		scale = channel.getValueAtTime(channel.scale_keys, now_time, looping, VEC3_ONE);
	}
}

vector<const char*> AnimationController::AnimationLayer::AdvanceTime(float delta_time)
{
	if (model == nullptr) return {};

	vector<const char*> result = prev_state.AdvanceTime(delta_time);
	vector<const char*> result2 = now_state.AdvanceTime(delta_time);
	result.insert(result.end(), result2.begin(), result2.end());

	transition_time += delta_time;
	if (transition_time >= transition_duration)
	{
		prev_state.anim_name = "";
		transition_time = 0.0f;
		transition_duration = 0.0f;
		if (model->getAnimClip(now_state.anim_name.c_str) != nullptr && now_state.now_time >= model->getAnimClip(now_state.anim_name.c_str)->duration && now_state.looping == false)
		{
			if (finished == false)
				{ result.push_back("finished"); }
			finished = true;
		}
	}

	return result;
}

void AnimationController::AnimationLayer::getTransformForBone(size_t bone_index, vec3& translation, quat& rotation, vec3& scale)
{
	if (model == nullptr) return;
	AnimationClip* now_clip = model->getAnimClip(now_state.anim_name.c_str);
	AnimationClip* prev_clip = model->getAnimClip(prev_state.anim_name.c_str);

	if (now_clip == nullptr && prev_clip == nullptr)
		return;

	vec3 now_translation = VEC3_ZERO;
	quat now_rotation = QUAT_IDENTITY;
	vec3 now_scale = VEC3_ONE;
	now_state.getTransformForBone(bone_index, now_translation, now_rotation, now_scale);

	if (prev_clip == nullptr || transition_duration <= 0.0f || transition_time >= transition_duration)
	{
		translation = now_translation * weight;
		rotation = quat::lerp(QUAT_IDENTITY, now_rotation, weight);
		scale = now_scale * weight;
		return;
	}

	vec3 prev_translation = VEC3_ZERO;
	quat prev_rotation = QUAT_IDENTITY;
	vec3 prev_scale = VEC3_ONE;
	prev_state.getTransformForBone(bone_index, prev_translation, prev_rotation, prev_scale);

	float blend = transition_time / transition_duration;

	translation = vec3::lerp(prev_translation, now_translation, blend) * weight;
	rotation = quat::lerp(QUAT_IDENTITY, quat::lerp(prev_rotation, now_rotation, blend), weight);
	scale = vec3::lerp(prev_scale, now_scale, blend) * weight;
}

void AnimationController::AnimationLayer::PlayAnimation(Model* model, const char* anim_name, bool loop, float speed, float transition)
{
	if (!model)
		return;

	prev_state = now_state;
//	now_state.anim_index = anim_index;
	now_state.anim_name = anim_name;

	now_state.prev_time = 0.0f;
	now_state.now_time = 0.0f;
	now_state.looping = loop;
	now_state.speed = speed;
	transition_duration = transition;
	transition_time = 0.0f;
	finished = false;
}

vector<const char*> AnimationController::CalcNextFrame(mat4 root_transform, double delta_time)
{
	if (model == nullptr)
	{
		for (size_t i = 0; i < bonedata.size(); ++i)
		{
			bonedata.get<ACM_PREV>(i) = MAT4_IDENTITY;
			bonedata.get<ACM_NOW>(i) = MAT4_IDENTITY;
		}

		return {};
	}

	vector<const char*> result;

	for (int i = 0; i < NUM_ANIM_LAYERS; ++i)
	{
		vector<const char*> layer_result = layers[i].AdvanceTime((float)delta_time * animspeed);
		result.insert(result.end(), layer_result.begin(), layer_result.end());

		if (layers[i].finished == true && i != 0)
		{
			layers[i].now_state.anim_name = "";
			layers[i].prev_state.anim_name = "";
			continue;
		}
	}

	// Calculate the bone transforms.
	for (size_t i = 0; i < bonedata.size(); ++i)
	{
		vec3 total_translation = VEC3_ZERO;
		quat total_rotation = QUAT_IDENTITY;
		vec3 total_scale = VEC3_ONE;

		for (int j = 0; j < NUM_ANIM_LAYERS; ++j)
		{
			vec3 translation = VEC3_ZERO;
			quat rotation = QUAT_IDENTITY;
			vec3 scale = VEC3_ONE;

			layers[j].getTransformForBone(i, translation, rotation, scale);

			total_translation += translation;
			total_rotation *= rotation;
			total_scale += scale;
		}
		
		total_rotation.normalize();

		// Apply the bone's local additive pose.
		total_translation += bonedata.get<AC_ADDPOS>(i);
		total_rotation *= bonedata.get<AC_ADDROT>(i);

		mat4 local_matrix = mat4::translation(total_translation) * mat4::rotation(total_rotation); // * mat4::scale(scale);

		// Include the bone-to-parent matrix.
		bonedata.get<ACM_NOW>(i) = model->getSkeleton().to_parent[i] * local_matrix;

		// If this bone has a parent (which it should if it's not 'root'), transform it according to its parent.
		int parent_index = model->getSkeleton().parent_index[i];
		if (parent_index >= 0 && parent_index < (int)i)
			{ bonedata.get<ACM_NOW>(i) = bonedata.get<ACM_NOW>(parent_index) * bonedata.get<ACM_NOW>(i); }

		// The only transformation left is the InvGlobalBindPose, which we'll do in the display update so we can use the current data right now.
		
		if (bonedata.get<AC_MOTIONSTATE>(i) != nullptr)
		{
			// Only do this if the rigidbody is kinematic!

			mat4 matrix = root_transform * model->getImportTransform() * bonedata.get<ACM_NOW>(i);

			vec3 world_position = (matrix * vec4(model->getSkeleton().collider_offset[i], 1) ).xyz;

//			vec3 world_position = matrix.extract_position();
			quat world_rotation = matrix.extract_rotation();

//			world_position = offset;

			btTransform world_transform;
			world_transform.setRotation({ world_rotation.x, world_rotation.y, world_rotation.z, world_rotation.w });
			world_transform.setOrigin({ world_position.x, world_position.y, world_position.z });
			bonedata.get<AC_MOTIONSTATE>(i)->setWorldTransform(world_transform);
		}
	
	}

	return result;
}

void AnimationController::CalcMatrices(float interpolation)
{
	if (model == nullptr)// || current_anim == nullptr)
	{
		for (size_t i = 0; i < bonedata.size(); ++i)
			{ bonedata.get<ACM_FINAL>(i) = MAT4_IDENTITY; }

		return;
	}

	// Interpolate and reconstruct the matrices.
	for (size_t i = 0; i < bonedata.size(); ++i)
	{
		vec3 translation = vec3::lerp(bonedata.get<ACM_PREV>(i).extract_position(), bonedata.get<ACM_NOW>(i).extract_position(), interpolation);
		quat rotation = quat::lerp(bonedata.get<ACM_PREV>(i).extract_rotation(), bonedata.get<ACM_NOW>(i).extract_rotation(), interpolation);
		mat4 bone_matrix = mat4::translation(translation) * mat4::rotation(rotation);

		bonedata.get<ACM_FINAL>(i) = bone_matrix * /* local bone scale */ model->getSkeleton().inv_bind_pose[i];
	}
}

void AnimationController::UploadMatrices()
{
	// With our bonedata calculated, we can send the data to the graphics card.
	if (skeleton_ubo_index == 0)
	{
		plog::error("Trying to upload an animation controller's bonedata before AnimationController has been initialized.\n");
		return;
	}

	Shader::UploadUniformBuffer(skeleton_ubo_index, sizeof(mat4) * bonedata.size(), bonedata.rawdata<ACM_FINAL>());
}

void AnimationController::getAnimatedBoneTransform(int bone_index, vec3& out_position, quat& out_rotation, const vmath::mat4& root_transform)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return;

	mat4 matrix = root_transform * model->getImportTransform() * bonedata.get<ACM_NOW>(bone_index);

	out_position = matrix.extract_position();
	out_rotation = matrix.extract_rotation();
}

// Additive pose

void AnimationController::setBoneAdditivePosition(int bone_index, vmath::vec3 pos)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return;

	bonedata.get<AC_ADDPOS>(bone_index) = pos;
}

void AnimationController::setBoneAdditivePositionLocal(int bone_index, vmath::vec3 pos)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return;

	bonedata.get<AC_ADDPOS>(bone_index) = pos;

	// Set the additive position for this bone's immediate children to be the inverse of their parent's.
	for (int i = bone_index; i < model->getSkeleton().num_bones; ++i)
	{
		if (model->getSkeleton().parent_index[i] == bone_index)
			{ bonedata.get<AC_ADDPOS>(i) = -pos; }
	}
}

vmath::vec3 AnimationController::getBoneAdditivePosition(int bone_index)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return VEC3_ZERO;

	return bonedata.get<AC_ADDPOS>(bone_index);
}

void AnimationController::ClearAdditivePositions()
{
	if (model == nullptr) return;

	for (int i = 0; i < (int)model->getSkeleton().num_bones; ++i)
		{ bonedata.get<AC_ADDPOS>(i) = VEC3_ZERO; }
}

void AnimationController::setBoneAdditiveRotation(int bone_index, quat rot)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return;

	bonedata.get<AC_ADDROT>(bone_index) = rot;
}

void AnimationController::setBoneAdditiveRotationLocal(int bone_index, quat rot)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return;

	bonedata.get<AC_ADDROT>(bone_index) = rot;

	// Set the additive position for this bone's immediate children to be the inverse of their parent's.
	for (int i = bone_index; i < model->getSkeleton().num_bones; ++i)
	{
		if (model->getSkeleton().parent_index[i] == bone_index)
			{ bonedata.get<AC_ADDROT>(i) = -rot; }
	}
}

vmath::quat AnimationController::getBoneAdditiveRotation(int bone_index)
{
	if (model == nullptr || bone_index < 0 || bone_index >= (int)model->getSkeleton().num_bones)
		return QUAT_IDENTITY;

	return bonedata.get<AC_ADDROT>(bone_index);
}

void AnimationController::ClearAdditiveRotations()
{
	if (model == nullptr) return;

	for (int i = 0; i < (int)model->getSkeleton().num_bones; ++i)
		{ bonedata.get<AC_ADDROT>(i) = QUAT_IDENTITY; }
}