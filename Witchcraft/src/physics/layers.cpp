#include "layers.h"

const std::map<std::string, PhysicsLayerEnum> PHYSICS_LAYER_MAP =
{
	{ "scene mesh",		PHYSICSLAYER_SCENEMESH },
	{ "scene object",	PHYSICSLAYER_SCENEOBJECT },
	{ "player",			PHYSICSLAYER_PLAYER },
	{ "camera",			PHYSICSLAYER_CAMERA },
	{ "player hitbox",	PHYSICSLAYER_PLAYERHITBOX },
	{ "npc hitbox",		PHYSICSLAYER_NPCHITBOX },
	{ "attack",			PHYSICSLAYER_ATTACK },
	{ "clutter",		PHYSICSLAYER_CLUTTER },
	{ "activator",		PHYSICSLAYER_ACTIVATOR },
	{ "trigger zone",	PHYSICSLAYER_TRIGGERZONE },

	{ "scenery",		PHYSICSLAYER_SCENERY },
	{ "hitbox",			PHYSICSLAYER_HITBOX },

	{ "scenery mask",	PHYSICSLAYER_SCENERYMASK },
	{ "clutter mask",	PHYSICSLAYER_CLUTTERMASK },
	{ "player mask",	PHYSICSLAYER_PLAYERMASK },
};