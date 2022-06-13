#include "scene.h"

#include "transform.h"
#include "name.h"
#include "script.h"
#include "renderable.h"
#include "animator.h"
#include "physical.h"

#include "physics/layers.h"

#include <rapidjson/document.h>
using namespace rapidjson;

#include "filesystem//file_manager.h"
using namespace std;

using namespace vmath;

namespace scene {

namespace {

	RenderableComponent component_renderable;
	AnimatorComponent component_animator(component_renderable);
	ColliderComponent component_collider(component_renderable);
	RigidBodyComponent component_rigidbody(component_collider);

	// This pointer stores all of the memory for every component.
	void* memory = nullptr;

} // namespace scene::<anon>

void Load(const char* sceneid, const char* entryid)
{
	char path[50];
	snprintf(path, 50, "%s%s%s", SCENE_DIRECTORY, sceneid, SCENE_EXTENSION);

	InFile file = filemanager::LoadSingleFile(path);
	if (!file.is_open())
		{ plog::error("Failed to find scene file '%s'.\n", sceneid); return; }

	string contents = file.contents();
	Document doc;

	if (doc.Parse(contents.c_str()).HasParseError())
		{ plog::error("Failed to parse scene file '%s'.\n", sceneid); return; }

	if (!doc.IsObject())
		{ plog::error("Parsing scene '%s'; document root should be an Object.\n", sceneid); return; }

	const auto& skybox = doc["skybox"];
	if (skybox.IsString())
	{
		renderer::LoadSkybox(skybox.GetString());
	}

	const auto& entities = doc["entities"];
	if (entities.IsObject())
	{
		// Before we can initialize the entities, we need to allocate memory for them.
		// To do so we need to count how many of each component exist.

		size_t num_names = 0;
		size_t num_transforms = 0;
		size_t num_renderables = 0;
		size_t num_animators = 0;
		size_t num_colliders = 0;
		size_t num_rigidbodies = 0;
		size_t num_scripts = 0;

		for (auto it = entities.MemberBegin(); it != entities.MemberEnd(); it++)
		{
			if (it->value.HasMember("name")) num_names++;
			if (it->value.HasMember("transform")) num_transforms++;
			if (it->value.HasMember("model")) num_renderables++;
			if (it->value.HasMember("animator")) num_animators++;
			if (it->value.HasMember("collider")) num_colliders++;
			if (it->value.HasMember("rigidbody")) num_rigidbodies++;
			if (it->value.HasMember("scripts")) num_scripts++;
		}

		// With the number of each component in hand, we can determine how much memory we'll need.

		size_t memsize =entity::name::size_of(num_names) +
						entity::transform::size_of(num_transforms) +
						//entity::renderable::size_of(num_renderables) +
						//entity::animator::size_of(num_animators) +
						//entity::collider::size_of(num_colliders) +
						//entity::rigidbody::size_of(num_rigidbodies) +
						entity::script::size_of(num_scripts);

		// Then we can allocate our memory.

		memory = new char[memsize];

		// Here we manage the pointers into said memory.

		void* memptr = memory;
		memptr = entity::name::Allocate(memptr, num_names);
		memptr = entity::transform::Allocate(memptr, num_transforms);
	//	memptr = entity::renderable::Allocate(memptr, num_renderables);
	//	memptr = entity::animator::Allocate(memptr, num_animators);
	//	memptr = entity::collider::Allocate(memptr, num_colliders);
	//	memptr = entity::rigidbody::Allocate(memptr, num_rigidbodies);
		memptr = entity::script::Allocate(memptr, num_scripts);
		
		// With the memory allocated, we can finally initialize the entities.
		for (auto entity = entities.MemberBegin(); entity != entities.MemberEnd(); ++entity)
		{
			int id = 0;
			sscanf(entity->name.GetString(), "%i", &id);

			const auto& name = entity->value["name"];
			if (name.IsString())
				{ entity::name::set(id, name.GetString()); }

			const auto& transform = entity->value["transform"];
			if (transform.IsObject())
			{
				vec3 pos = VEC3_ZERO; quat rot = QUAT_IDENTITY; vec3 scl = VEC3_ONE;

				if (transform.HasMember("position")) {
					const auto& position = transform["position"];
					if (position.IsArray())
						{ for (SizeType i = 0; i < 3; ++i) { pos.data[i] = position[i].GetFloat(); } }
				}

				if (transform.HasMember("rotation")) {
					const auto& rotation = transform["rotation"];
					if (rotation.IsArray())
						{ vec3 euler; for (SizeType i = 0; i < 3; ++i) { euler.data[i] = TO_RADIANS * rotation[i].GetFloat(); } rot = quat::euler(euler); }
				}

				if (transform.HasMember("scale")) {
					const auto& scale = transform["scale"];
					if (scale.IsArray())
						{ for (SizeType i = 0; i < 3; ++i) { scl.data[i] = scale[i].GetFloat(); } }
				}

				entity::transform::add(id, pos, rot, scl);
			}

			if (entity->value.HasMember("model")) {
				const auto& model = entity->value["model"];
				if (model.IsString())
				{
					component_renderable.AddEmpty(id);
					component_renderable.LoadModel(id, model.GetString());
			}	}

			if (entity->value.HasMember("animator")) {
				const auto& animator = entity->value["animator"];
				if (!animator.IsNull())
					{ component_animator.AddTo(id); }
				if (animator.IsObject())
				{
					const auto& import = animator["import"];
					if (import.IsArray())
					{
						for (auto it = import.Begin(); it != import.End(); ++it)
							{ if (it->IsString()) component_animator.ImportAnimations(id, it->GetString()); }
					}
			}	}

			if (entity->value.HasMember("collider")) {
				const auto& collider = entity->value["collider"];
				if (collider.IsObject())
				{
					ColliderTypeEnum collider_type = COLLIDER_NULL; const auto& shape = collider["shape"];
					if (shape.IsString())
					{
						if (COLLIDER_TYPE_STR_TO_ENUM.count(shape.GetString()) == 0)
							plog::error("Invalid collider shape string: '%s'.\n", shape.GetString());
						else
							collider_type = COLLIDER_TYPE_STR_TO_ENUM.at(shape.GetString());
					}
					
					vec3 offset = VEC3_ZERO; const auto& _offset = collider["offset"];
					if (_offset.IsArray()) { for (SizeType i = 0; i < 3; ++i) { offset.data[i] = _offset[i].GetFloat(); } }
					
					if (collider.HasMember("size")) {
						const auto& size = collider["size"];
						if (size.IsArray())
						{
							vec3 half_extends = VEC3_ZERO;
							for (SizeType i = 0; i < 3; ++i)
								{ half_extends.data[i] = size[i].GetFloat(); }
							
							component_collider.MakeSimpleShape(id, { collider_type, half_extends }, offset);
					} }
					else
					{
						float radius = 0.0f, height = 0.0f;
	
						if (collider.HasMember("radius")) {
							const auto& _radius = collider["radius"];
							if (_radius.IsNumber()) radius = _radius.GetFloat();
						}
	
						if (collider.HasMember("height")) {
							const auto& _height = collider["height"];
							if (_height.IsNumber()) height = _height.GetFloat();
						}

						component_collider.MakeSimpleShape(id, { collider_type, {radius, height, 0.0f} }, offset);
					}
				}
				else if (collider.IsString())
				{
					if (strcmp(collider.GetString(), "create_from_model") == 0)
						component_collider.MakeColliderFromModel(id);
				}
			}

			if (entity->value.HasMember("rigidbody")) {
				const auto& rigidbody = entity->value["rigidbody"];
				if (rigidbody.IsObject())
				{
					float mass = 0.0f; const auto& _mass = rigidbody["mass"];
					if (_mass.IsNumber()) mass = _mass.GetFloat();

					bool is_kinematic = false; const auto& kinematic = rigidbody["kinematic"];
					if (kinematic.IsBool()) is_kinematic = kinematic.GetBool();

					bool is_trigger = false; const auto& trigger = rigidbody["trigger"];
					if (trigger.IsBool()) is_trigger = trigger.GetBool();

					int collision_group = PHYSICSLAYER_DEFAULT; const auto& group = rigidbody["group"];
					if (group.IsString())
					{
						if (PHYSICS_LAYER_MAP.count(group.GetString()) == 0)
							plog::error("Invalid rigidbody group string: '%s'.\n", group.GetString());
						else
							collision_group = PHYSICS_LAYER_MAP.at(group.GetString());
					}

					int collision_mask = PHYSICSLAYER_DEFAULT; const auto& mask = rigidbody["mask"];
					if (mask.IsArray())
					{
						for (auto it = mask.Begin(); it != mask.End(); ++it)
						{
							if (PHYSICS_LAYER_MAP.count(it->GetString()) == 0)
								plog::error("Invalid rigidbody group string: '%s'.\n", it->GetString());
							else
								collision_mask |= PHYSICS_LAYER_MAP.at(it->GetString());
						}
					}

					component_rigidbody.Create(id, mass, is_kinematic, is_trigger, collision_group, collision_mask);
			}	}

			if (entity->value.HasMember("scripts")) {
				const auto& scripts = entity->value["scripts"];
				if (scripts.IsObject())
				{
					for (auto script = scripts.MemberBegin(); script != scripts.MemberEnd(); ++script)
					{
						entity::script::attach(id, script->name.GetString(), script);
					}
			}	}
		}
	}
}

void Clear()
{
//	entity::name::Clear();
	entity::transform::Clear();
//	entity::renderable::Clear();
//	etc...

	if (memory)
	{
		delete[] memory;
		memory = nullptr;
	}
}

entity::ID SpawnEntity()
	{ return entity::Create(0); }

void DestroyEntity(entity::ID id)
{
	entity::script::detachAll(id);

	component_rigidbody.Delete(id);
	component_collider.Delete(id);
	component_animator.RemoveEntry(id);
	component_renderable.Remove(id);
	entity::transform::remove(id);
	entity::name::remove(id);
	entity::Destroy(id);
}

RenderableComponent& getRenderableComponent() { return component_renderable; }
AnimatorComponent& getAnimatorComponent() { return component_animator; }
ColliderComponent& getColliderComponent() { return component_collider; }
RigidBodyComponent& getRigidBodyComponent() { return component_rigidbody; }


} // namespace scene