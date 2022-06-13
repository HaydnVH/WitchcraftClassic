#ifndef HVH_WC_GRAPHICS_LIGHT_H
#define HVH_WC_GRAPHICS_LIGHT_H

#include "math/vmath.h"

#pragma pack(push, 1)
struct PointLight
{
	vmath::vec4 position; // w is radius
	vmath::vec4 color; // a is unused
	float ambient;
	float diffuse;
	float specular;
	float padding;
};

struct DirectionalLight
{
	vmath::vec4 direction; // w is unused
	vmath::vec4 diffuse_color; // a is specular multiplier
	vmath::vec4 ambient_color; // a is unused
};

constexpr const int MAX_CASCADES = 5;
constexpr const int MAX_DIRECTIONAL_LIGHTS = 4;
constexpr const int MAX_POINT_LIGHTS = 512;
constexpr const int MAX_LOCAL_POINT_LIGHTS = 16;

struct ShadowingDirectionalLight
{
	DirectionalLight l;
	vmath::mat4 cascade_matrices[MAX_CASCADES];
	vmath::vec4 cascade_boundries[MAX_CASCADES - 1];
	int num_cascades;
	int padding[3]; // unused
};

struct UniformBufferLighting
{
	ShadowingDirectionalLight sunlight;

	DirectionalLight dir_lights[MAX_DIRECTIONAL_LIGHTS];
	PointLight point_lights[MAX_POINT_LIGHTS];
	int num_dir_lights_used;
	int padding[3];
};
#pragma pack(pop)

#endif // HVH_WC_GRAPHICS_LIGHT_H