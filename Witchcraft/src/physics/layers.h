#ifndef HVH_WC_PHYSICS_LAYERS_H
#define HVH_WC_PHYSICS_LAYERS_H

#include <map>
#include <string>

enum PhysicsLayerEnum
{
	PHYSICSLAYER_DEFAULT = 1 << 0,
	PHYSICSLAYER_SCENEMESH = 1 << 1,	// The concave triangle mesh(es) which make up the level itself.
	PHYSICSLAYER_SCENEOBJECT = 1 << 2,	// Static (or kinematic) objects which make up the scene, other than concave meshes.
	PHYSICSLAYER_PLAYER = 1 << 3,		// The kinematic object associated with the player.
	PHYSICSLAYER_CAMERA = 1 << 4,		// Is the camera actually a physical object? Let's reserve a bit for it just in case.
	PHYSICSLAYER_PLAYERHITBOX = 1 << 5,	// Hitboxes associated with the player.
	PHYSICSLAYER_NPCHITBOX = 1 << 6,	// Hitboxes associated with NPCs and enemies.
	PHYSICSLAYER_ATTACK = 1 << 7,		// Hurtboxes associated with attacks.
	PHYSICSLAYER_CLUTTER = 1 << 8,		// Dynamic rigidbodies around the scene.
	PHYSICSLAYER_ACTIVATOR = 1 << 9,	// Objects which you can interact with in first-person mode.
	PHYSICSLAYER_TRIGGERZONE = 1 << 10,	// Trigger volumes (activators or otherwise).

	// Some handy compound layers.
	PHYSICSLAYER_SCENERY = PHYSICSLAYER_SCENEMESH | PHYSICSLAYER_SCENEOBJECT,
	PHYSICSLAYER_HITBOX = PHYSICSLAYER_PLAYERHITBOX | PHYSICSLAYER_NPCHITBOX,
 
	// Preset collision masks for various object types.
	PHYSICSLAYER_SCENERYMASK = PHYSICSLAYER_PLAYER | PHYSICSLAYER_CAMERA | PHYSICSLAYER_CLUTTER,
	PHYSICSLAYER_CLUTTERMASK = PHYSICSLAYER_SCENERY | PHYSICSLAYER_PLAYER | PHYSICSLAYER_ATTACK | PHYSICSLAYER_CLUTTER,
	PHYSICSLAYER_PLAYERMASK = PHYSICSLAYER_SCENERY | PHYSICSLAYER_CLUTTER | PHYSICSLAYER_TRIGGERZONE,
};

extern const std::map<std::string, PhysicsLayerEnum> PHYSICS_LAYER_MAP;

#endif // HVH_WC_PHYSICS_LAYERS_H