#include "model.h"

#include "filesystem/file_manager.h"
#include "sys/printlog.h"

#include <string>
#include <map>
using namespace std;
using namespace vmath;

#include <btBulletDynamicsCommon.h>

namespace {


} // namespace <anon>

void Model::Clear()
{
	refcount = 0;
	import_transform = MAT4_IDENTITY;

	// Geometry
	{
		geom.buffer_size = 0;
		if (geom.buffer_ptr)
			{ delete[] geom.buffer_ptr; }
		geom.buffer_ptr = nullptr;

		geom.num_vertices = 0;
		geom.vertex_format = 0;
		geom.positions_ptr = nullptr;
		geom.surface_ptr = nullptr;
		geom.skin_ptr = nullptr;

		geom.num_indices = 0;
		geom.index_size = 0;
		geom.indices_ptr = nullptr;
#ifndef MODEL_CONVERTER
		geom.geometry.Clean();
#endif
	}

	// Meshes
	{
		meshes.count = 0;
		meshes.start = nullptr;
		meshes.primcount = nullptr;
		meshes.material_id = nullptr;
		meshes.material_ptr = nullptr;
	}

	// Skeleton
	{
		skeleton.num_bones = 0;
		skeleton.bone_name = nullptr;
		skeleton.inv_bind_pose = nullptr;
		skeleton.to_parent = nullptr;
		skeleton.parent_index = nullptr;
		skeleton.collider = nullptr;
		skeleton.collider_offset = nullptr;
		skeleton.collider_flags = nullptr;

		skeleton.bone_map.clear();
	}

	// Collision
	{
		collision.type = COLLIDER_NULL;
		collision.offset_position = vmath::VEC3_ZERO;
		collision.offset_rotation = vmath::QUAT_IDENTITY;

		// If num_simple_shapes is greater than 1, a compound collider is created.
		collision.num_simple_shapes = 0;
		collision.simple_collider_shapes = nullptr;

		// If we don't use simple shapes, we can define a triangle collision mesh.
		collision.num_vertices = 0;
		collision.vertices_ptr = nullptr;
		collision.num_indices = 0;
		collision.indices_ptr = nullptr;
	}

	// Animations
	{
		// Destruct the animation clips properly, since they wont be destructed when the persistant buffer is deallocated.
		for (int i = 0; i < animations.count; ++i)
			{ animations.clip[i].~AnimationClip(); }

		animations.count = 0;
		animations.clip = nullptr;

		animations.anim_map.clear();

#ifndef MODEL_CONVERTER
		for (auto& it : animations.imports)
			{ Model::Unload(it.first.c_str); }
		animations.imports.clear();
#endif
	}

	// Delete the persistant buffer
	persistant_buffer_size = 0;
	if (persistant_buffer)
		{ delete[] persistant_buffer; }
	persistant_buffer = nullptr;

	// Ragdoll
	num_ragdoll_bones = 0;
	if (ragdoll_buffer)
	{
		delete[] ragdoll_buffer;
		ragdoll_buffer = nullptr;
	}
	ragbone_bones = nullptr;
	ragbone_parents = nullptr;
	ragbone_colliders = nullptr;
	ragbone_offsets = nullptr;
	ragbone_flags = nullptr;
}

#ifndef MODEL_CONVERTER
#include "renderer.h"
#include "material.h"

map<string, Model*> loaded_models;

Model* Model::Load(const char* filename)
{
	Model* result = nullptr;

	// Check to see if the model's already been loaded.
	if (loaded_models.count(filename) != 0)
	{
		result = loaded_models[filename];
		result->refcount++;
		return result;
	}

	// Open the file.
	char full_filename[64];
	snprintf(full_filename, 64, "%s%s", MODEL_FOLDER, filename);
	InFile file = filemanager::LoadSingleFile(full_filename, ios::binary);
	if (file.is_open() == false)
	{
		plog::error("Failed to open model file '%s'.\n", filename);
		return nullptr;
	}

	result = new Model();

	// Load the file.
	const char* err1 = nullptr, *err2 = nullptr;
	if ((err1 = result->LoadBin(file)) != nullptr)
	{
		file.seekg(0);
		if ((err2 = result->LoadXML(file)) != nullptr)
		{
			plog::error("Failed to load model file '%s':\n", filename);
			if (err1) plog::errmore("%s\n", err1);
			if (err2) plog::errmore("%s\n", err2);
			delete result;
			return nullptr;
		}
	}

	if (result->geom.num_vertices > 0)
	{
		// Send the model data to the graphics card.
		result->geom.geometry.Load(GEOMETRY_PRIMITIVE_TRIANGLES,
			result->geom.buffer_ptr, (VertexFormatFlags)result->geom.vertex_format, result->geom.num_vertices,
			result->geom.indices_ptr, result->geom.index_size, result->geom.num_indices);
	}
	

	// Load the materials.
	for (size_t i = 0; i < result->meshes.count; ++i)
		{ result->meshes.material_ptr[i] = Material::Load(result->meshes.material_id[i].c_str); }

//	plog::info("Loaded '%s'; geometry buffer size: %i; persistant buffer size: %i\n", filename, result->geom.buffer_size, result->persistant_buffer_size);

	result->refcount = 1;
	loaded_models[filename] = result;
	return result;
}

void Model::Unload(const char* filename)
{
	if (loaded_models.count(filename) == 0)
		return;

	Model* ptr = loaded_models[filename];
	ptr->refcount--;
	if (ptr->refcount <= 0)
	{
		for (size_t i = 0; i < ptr->meshes.count; ++i)
		{
			Material::Unload(ptr->meshes.material_id[i].c_str);
			ptr->meshes.material_ptr[i] = nullptr;
		}

		delete ptr;
		loaded_models.erase(filename);
	}
}

void Model::ImportAnimations(const char* filename)
{
	Model* ptr = Model::Load(filename);
	if (!ptr)
	{
		plog::error("Couldn't import animations from '%s'.\n", filename);
		return;
	}

	animations.imports[filename] = ptr;

	for (int32_t i = 0; i < ptr->animations.count; ++i)
	{
		AnimationClip* anim = &ptr->animations.clip[i];

		// We can't have two animations with the same name, so if we find one in the imported model, skip it.
		if (animations.anim_map.count(anim->name) != 0)
		{
			plog::warning("Animation imported from '%s' has the same name as an animation already available to this model: '%s'.\n", filename, anim->name.c_str);
			continue;
		}

		animations.anim_map[anim->name] = anim;

	} // for each animation
}

void Model::Draw(entity::ID transformid)
{
	for (size_t i = 0; i < meshes.count; ++i)
	{
		if (meshes.material_ptr[i] == nullptr)
			{ continue; }

		renderer::SubmitRenderable(transformid, &geom.geometry, meshes.material_ptr[i], meshes.start[i], meshes.primcount[i]);
	}
}

void Model::AssignMaterial(int mesh_id, const char* material_name)
{
	if (mesh_id > meshes.count)
		return;

	Material::Unload(meshes.material_id[mesh_id].c_str);
	meshes.material_id[mesh_id] = material_name;
	meshes.material_ptr[mesh_id] = Material::Load(material_name);
}
#endif // MODEL_CONVERTER