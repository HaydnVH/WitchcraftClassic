#ifndef HVH_WC_GRAPHICS_ANIMATIONCONTROLLER_H
#define HVH_WC_GRAPHICS_ANIMATIONCONTROLLER_H

#include "math/vmath.h"
#include "tools/structofarrays.h"
#include "tools/fixedstring.h"
#include <vector>
#include <memory>

#include "scene/entity.h"

constexpr const int NUM_ANIM_LAYERS = 8;

class Model;
struct AnimationClip;
class ShaderManager;

class PhysicsSystem;
class btRigidBody;
class btMotionState;

class AnimationController
{
public:
	AnimationController(Model* model, entity::ID id);
	~AnimationController();

	static void initShaders();

	void Flip();
	std::vector<const char*> CalcNextFrame(vmath::mat4 root_transform, double delta_time = (1.0 / 30.0));
	void CalcMatrices(float interpolation);
	void UploadMatrices();

//	int findAnimIndex(const char* anim_name);
	void PlayAnimation(int layer, const char* anim_name, bool loop = false, float speed = 1.0f, float transition = 0.0f)
	{
		if (layer >= 0 && layer < NUM_ANIM_LAYERS) 
			{ layers[layer].PlayAnimation(model, anim_name, loop, speed, transition); }
	}
	const char* getAnimName(int layer);

	void setAnimLayerWeight(int layer, float val)
		{ if (layer >= 0 && layer < NUM_ANIM_LAYERS) layers[layer].weight = val; }
	float getAnimLayerWeight(int layer)
		{ if (layer >= 0 && layer < NUM_ANIM_LAYERS) return layers[layer].weight; else return 0.0f; }

	float getAnimSpeed()
		{ return animspeed; }
	void setAnimSpeed(float new_speed)
		{ if (new_speed < 0.0f) animspeed = 0.0f; else animspeed = new_speed; }

	bool isLooping(int layer)
		{ if (layer < 0 || layer >= NUM_ANIM_LAYERS) return false; else return layers[layer].now_state.looping; }

	void setLooping(int layer, bool new_looping)
	{
		if (layer < 0 || layer >= NUM_ANIM_LAYERS)
			return;

		layers[layer].now_state.looping = new_looping;
		if (layers[layer].now_state.looping && layers[layer].finished)
			{ layers[layer].finished = false; }
	}

	float getAnimDuration(int layer);
	float getAnimTime(int layer)
		{ if (layer < 0 || layer >= NUM_ANIM_LAYERS) return 0.0f; else return layers[layer].now_state.now_time; }

	void getAnimatedBoneTransform(int bone_index, vmath::vec3& out_position, vmath::quat& out_rotataion, const vmath::mat4& root_transform = vmath::MAT4_IDENTITY);

	void setBoneAdditivePosition(int bone_index, vmath::vec3 pos);
	void setBoneAdditivePositionLocal(int bone_index, vmath::vec3 pos);
	vmath::vec3 getBoneAdditivePosition(int bone_index);
	void ClearAdditivePositions();

	void setBoneAdditiveRotation(int bone_index, vmath::quat rot);
	void setBoneAdditiveRotationLocal(int bone_index, vmath::quat rot);
	vmath::quat getBoneAdditiveRotation(int bone_index);
	void ClearAdditiveRotations();

private:

	struct AnimationState
	{
		AnimationState()
		:	model(nullptr),
			anim_name(""),
			prev_time(0),
			now_time(0),
			speed(1),
			looping(false)
		{}

		Model* model;
//		int anim_index;
		FixedString<32> anim_name;
//		AnimationClip* clip;
		float prev_time;
		float now_time;
		float speed;
		bool looping;

		void setmodel(Model* modelptr) { model = modelptr; }

		std::vector<const char*> AdvanceTime(float delta_time);
		void getTransformForBone(size_t bone_index, vmath::vec3& translation, vmath::quat& rotation, vmath::vec3& scale);
	};

	struct AnimationLayer
	{
		AnimationLayer()
		:	model(nullptr),
			transition_duration(0.0f),
			transition_time(0.0f),
			finished(false),
			weight(0.0f)
		{}

		Model* model;
		AnimationState prev_state;
		AnimationState now_state;
		float transition_duration;
		float transition_time;
		bool finished;

		float weight;

		bool mask[256];
		bool mask_blend;

		void setmodel(Model* modelptr) { model = modelptr; prev_state.setmodel(modelptr); now_state.setmodel(modelptr); }

		std::vector<const char*> AdvanceTime(float delta_time);
		void getTransformForBone(size_t bone_index, vmath::vec3& translation, vmath::quat& rotation, vmath::vec3& scale);
		void PlayAnimation(Model* model, const char* anim_name, bool loop, float speed, float transition);
	};

	AnimationLayer layers[NUM_ANIM_LAYERS];

	Model* model;
	float animspeed;

	enum PerBoneSoaEnum
	{
		ACM_PREV = 0,
		ACM_NOW,
		ACM_FINAL,
		AC_ADDPOS,
		AC_ADDROT,
		AC_RIGIDBODY,
		AC_MOTIONSTATE,
	};
	StructOfArrays<vmath::mat4, vmath::mat4, vmath::mat4, vmath::vec3, vmath::quat, std::shared_ptr<btRigidBody>, std::shared_ptr<btMotionState>> bonedata;
};

#endif // HVH_WC_GRAPHICS_ANIMATIONCONTROLLER_H