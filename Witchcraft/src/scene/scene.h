#ifndef HVH_WC_SCENE_SCENE_H
#define HVH_WC_SCENE_SCENE_H

#include "entity.h"

#include "transform.h"
#include "name.h"
#include "script.h"
#include "renderable.h"
#include "animator.h"
#include "physical.h"

#include "graphics/renderer.h"

namespace scene {

	constexpr const char* SCENE_DIRECTORY = "scenes/";
	constexpr const char* SCENE_EXTENSION = ".scene.json";
	
	void Load(const char* sceneid, const char* entryid);
	void Clear();

	entity::ID SpawnEntity();
	void DestroyEntity(entity::ID id);

	RenderableComponent& getRenderableComponent();
	AnimatorComponent& getAnimatorComponent();
	ColliderComponent& getColliderComponent();
	RigidBodyComponent& getRigidBodyComponent();

	void InitLua();

} // namespace scene


#endif // HVH_WC_SCENE_SCENE_H