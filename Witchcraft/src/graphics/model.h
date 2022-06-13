#ifndef HVH_WC_GRAPHICS_MODEL_H
#define HVH_WC_GRAPHICS_MODEL_H

#ifndef MODEL_CONVERTER
class Material;
#include "geometry.h"
#include "scene/entity.h"
#endif // MODEL_CONVERTER

#include "vertex.h"
#include "math/vmath.h"
#include "tools/structofarrays.h"
#include "tools/fixedstring.h"

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <iostream>

#include "physics/collider.h"
class btCollisionShape;

constexpr const char* MODEL_FOLDER = "models/";

constexpr const char* BINFILE_MAGIC = "WCM";
constexpr uint32_t CURRENT_FILE_VERSION = 1;

struct AnimationChannel
{
	FixedString<32> bone_name;
	StructOfArrays<float, vmath::vec3> position_keys;
	StructOfArrays<float, vmath::quat> rotation_keys;
	StructOfArrays<float, vmath::vec3> scale_keys;

	bool isEmpty()
	{
		return (
			position_keys.size() == 0 &&
			rotation_keys.size() == 0 &&
			scale_keys.size() == 0);
	}

	template <typename T>
	T getValueAtTime(StructOfArrays<float, T>& keys, float now, bool loop, T default_val) const
	{
		if (keys.size() == 0)
			return default_val;

		size_t prev_frame = 0;
		size_t next_frame = 0;

		// Search for the frame that corresponds to 'now'.  The result will be 'greater than or equal to' now.
		size_t it = keys.find_sorted<0>(now);

		// If the time we're searching for is after any existing frame,
		if (it == keys.size())
		{
			prev_frame = keys.size() - 1;
			next_frame = prev_frame;
		}

		// If the time we found is exactly equal to right now (very unlikely, but possible),
		else if (keys.get<0>(it) == now)
		{
			// both prev_ and next_frame should refer to the frame we found.
			prev_frame = it;
			next_frame = it;
		}

		// If the frame we've found is somewhere in the middle of the timeline,
		else
		{
			// next_frame should point to the frame we found, and prev_frame should be the frame right before it.
			next_frame = it;
			if (next_frame == 0)
			{
				// This is only possible if the first frame's time key is greater than 0 seconds.
				if (loop)
					prev_frame = keys.size() - 1;
				else
					prev_frame = next_frame;
			}
			else
			{
				prev_frame = next_frame - 1;
			}
		}

		// If prev_frame and next_frame refer to the same frame, we simply return the value at that frame (and also avoid a divide-by-zero error).
		if (prev_frame == next_frame)
		{
			return keys.get<1>(prev_frame);
		}
		else
		{
			// Otherwise we have to use the time to calculate an interpolation between the two.
			float delta_time = keys.get<0>(next_frame) - keys.get<0>(prev_frame);
			float blend_factor = (now - keys.get<0>(prev_frame)) / delta_time;
			return T::lerp(keys.get<1>(prev_frame), keys.get<1>(next_frame), blend_factor);
		}
	}
};

struct AnimationClip
{
	FixedString<32> name;
	float duration;
	size_t num_used_channels;
	StructOfArrays<float, FixedString<32>> events;
	std::vector<AnimationChannel> channels;

	std::vector<const char*> getEventsBetweenFrames(float prev_time, float this_time)
	{
		std::vector<const char*> result;

		for (size_t i = 0; i < events.size(); ++i)
		{
			if (events.get<0>(i) < prev_time)
				continue;

			if (events.get<0>(i) > this_time)
				break;

			result.push_back(events.get<1>(i).c_str);
		}

		return result;
	}
};

class Material;

class Model
{
	friend class AnimationController;

public:
	Model() {};
	~Model()
		{ Clear(); }

	void Clear();

	const char* LoadXML(std::istream& file);
	const char* SaveXML(std::ostream& file);

	const char* LoadBin(std::istream& file) { return ""; }
	const char* SaveBin(std::ostream& file) { return ""; }

#ifdef MODEL_CONVERTER
	bool LoadFBX(const char* filename, const char* password = nullptr);
#endif

#ifndef MODEL_CONVERTER
	static Model* Load(const char* filename);
	static void Unload(const char* filename);

	void ImportAnimations(const char* filename);

	void Draw(entity::ID transform_id);

	int numMeshes() { return (int)meshes.count; }
	void AssignMaterial(int mesh_id, const char* material_name);
#endif

	AnimationClip* getAnimClip(const char* anim_name)
	{
		if (animations.anim_map.count(anim_name) == 0)
			return nullptr;
		else
			return animations.anim_map[anim_name];
	}

	struct Skeleton {

		int32_t num_bones = 0;
		FixedString<32> * bone_name = nullptr;
		vmath::mat4* inv_bind_pose = nullptr;
		vmath::mat4* to_parent = nullptr;
		int32_t* parent_index = nullptr;
		btCollisionShape** collider = nullptr;
		vmath::vec3* collider_offset = nullptr;
		uint32_t* collider_flags = nullptr;

		std::unordered_map<FixedString<32>, int32_t> bone_map;
		static constexpr size_t bonesize = sizeof(FixedString<32>) + sizeof(vmath::mat4) + sizeof(vmath::mat4) + sizeof(int32_t) + sizeof(btCollisionShape*) + sizeof(vmath::vec3) + sizeof(uint32_t);

		enum RagdollFlagsEnum
		{
			RD_DEFAULTDYNAMIC = 1 << 0
		};

	};
	const Skeleton& getSkeleton()
		{ return skeleton; }

	struct Collision {

		ColliderTypeEnum type = COLLIDER_NULL;
		vmath::vec3 offset_position = vmath::VEC3_ZERO;
		vmath::quat offset_rotation = vmath::QUAT_IDENTITY;

		// If num_simple_shapes is greater than 1, a compound collider is created.
		uint32_t num_simple_shapes = 0;
		SimpleShapeCollider* simple_collider_shapes = nullptr;

		// If we don't use simple shapes, we can define a triangle collision mesh.
		int32_t num_vertices = 0;
		VertexPosition* vertices_ptr = nullptr;
		int32_t num_indices = 0;
		int32_t* indices_ptr = nullptr;

	};
	const Collision& getCollision()
		{ return collision; }

	vmath::mat4 getImportTransform()
		{ return import_transform; }

	

private:

	// Multiple entities might share the same model, and we'd like to not have to load the file more than once.
	int32_t refcount = 0;

	// Import Transform
	vmath::mat4 import_transform = vmath::MAT4_IDENTITY;

	// Geometry
	struct Geom {

		uint32_t buffer_size = 0;
		void* buffer_ptr = nullptr;

		int32_t num_vertices = 0;
		uint32_t vertex_format = 0;
		VertexPosition* positions_ptr = nullptr;
		VertexSurface* surface_ptr = nullptr;
		VertexSkin* skin_ptr = nullptr;

		int32_t num_indices = 0;
		uint32_t index_size = 0;
		void* indices_ptr = nullptr;

#ifndef MODEL_CONVERTER
		Geometry geometry;
#endif
	} geom;

	// When a model is loaded, two contiguous blocks of memory are allocated;
	// one for the mesh's geometry (above), which is freed after being uploaded to the GPU,
	// and one for everything else, which we keep a pointer to here and free when the model is destroyed.
	uint32_t persistant_buffer_size = 0;
	void* persistant_buffer = nullptr;

	// Meshes
	struct Meshes {

		int32_t count = 0;
		int32_t* start = nullptr;
		int32_t* primcount = nullptr;
		FixedString<32>* material_id = nullptr;
		Material** material_ptr = nullptr;

		static constexpr size_t meshsize = sizeof(int32_t) + sizeof(int32_t) + sizeof(FixedString<32>) + sizeof(Material*);

	} meshes;

	// Skeleton
	Skeleton skeleton;

	// Collision
	Collision collision;

	// Animations
	struct Animations {

		int32_t count = 0;
		AnimationClip* clip;

		std::unordered_map<FixedString<32>, AnimationClip*> anim_map;
		std::map<FixedString<32>, Model*> imports;

	} animations;

	// Ragdoll
	enum RagdollEnum
	{
		RAGDOLL_BONE = 0,
		RAGDOLL_PARENT,
		RAGDOLL_COLLIDER,
		RAGDOLL_OFFSET,
		RAGDOLL_FLAGS,

		RAGDOLL_DEFAULTDYNAMIC = 1 << 0,
	};

	uint32_t num_ragdoll_bones = 0;
	void* ragdoll_buffer = nullptr;
	uint32_t* ragbone_bones= nullptr;
	uint32_t* ragbone_parents = nullptr;
	SimpleShapeCollider* ragbone_colliders = nullptr;
	vmath::vec3* ragbone_offsets = nullptr;
	uint32_t* ragbone_flags = nullptr;

//	StructOfArrays<int, int, SimpleShapeCollider, vmath::vec3, int> ragdoll_bones;

	struct BinFileHeader
	{
		char magic[4]; // WCM\0
		uint32_t num_vertices, num_indices;
		uint32_t vertex_buffer_size, index_buffer_size, skeleton_buffer_size;
		uint16_t file_version;
		uint16_t collider_numvertices, collider_numindices, collider_numshapes;
		uint8_t vertex_format, index_size, num_meshes, num_bones, num_animations;
		uint8_t collider_type;
		uint8_t padding[2];
	};
};

#endif // HVH_WC_GRAPHICS_MODEL_H