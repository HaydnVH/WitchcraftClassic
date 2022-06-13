R"==(
#begin common

#ifndef SKIP_LIGHTING

#define MAX_CASCADES 5
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 512
#define MAX_LOCAL_POINT_LIGHTS 16

struct PointLight
{
	vec4 position; // position.w is the radius of the light
	vec4 color; // color.a is unused for now
	float ambient;
	float diffuse;
	float specular;
	float padding; // unused
};

struct DirectionalLight
{
	vec4 direction; // _.w is unused for now
	vec4 diffuse_color; // _.a is unused for now
	vec4 ambient_color;
};

// TODO: Can we use clever integer packing to reduce the light structures from 48 bytes to 32?
// Worth looking into, at any rate.

struct ShadowingDirectionalLight
{
	DirectionalLight l;

	mat4 cascade_matrices[MAX_CASCADES];
	vec4 cascade_boundries[MAX_CASCADES-1];
	int num_cascades;
};

layout (std140) uniform LIGHTING
{
	ShadowingDirectionalLight sunlight;
	DirectionalLight dir_lights[MAX_DIRECTIONAL_LIGHTS];
	PointLight point_lights[MAX_POINT_LIGHTS];
	int num_dir_lights_used;
};

#endif // SKIP_LIGHTING
	
#end // common ////////////////////////////////////////////
#begin vertex shader //////////////////////////////////////

#ifndef SKIP_LIGHTING

out vec3 vert_world_pos;
out vec4 vert_lightspace_pos[MAX_CASCADES];
out float vert_viewspace_z;

// This function needs to be called in the vertex shader for each cascade of the sunlight's shadow map.
void CalcVertexLightVals(mat4 mat_transform, vec3 position)
{
	vert_world_pos = (mat_transform * vec4(position, 1)).xyz;
	vert_viewspace_z = (view * vec4(vert_world_pos, 1)).z;
	
	for (int i = 0; i < 5; ++i)
	{
		vert_lightspace_pos[i] = sunlight.cascade_matrices[i] * mat_transform * vec4(position, 1);
	}
}

#endif // SKIP_LIGHTING

#end // vertex shader /////////////////////////////////////
#begin fragment shader ////////////////////////////////////

#ifndef SKIP_LIGHTING

in vec3 vert_world_pos;
in vec4 vert_lightspace_pos[MAX_CASCADES];
in float vert_viewspace_z;

uniform sampler2DShadow sunlight_shadow_map;
//uniform sampler2D random_texture;

struct LightIndices
{
	int index[MAX_LOCAL_POINT_LIGHTS];
	int num_lights;
};
uniform LightIndices local_lighting;


//vec2 poisson_disk[4] = vec2[](
 // vec2( -0.94201624, -0.39906216 ),
 // vec2( 0.94558609, -0.76890725 ),
 // vec2( -0.094184101, -0.92938870 ),
 // vec2( 0.34495938, 0.29387760 )
//);

vec2 poissonDisk[64] = vec2[](
	vec2(-0.613392, 0.617481),
	vec2(0.170019, -0.040254),
	vec2(-0.299417, 0.791925),
	vec2(0.645680, 0.493210),
	vec2(-0.651784, 0.717887),
	vec2(0.421003, 0.027070),
	vec2(-0.817194, -0.271096),
	vec2(-0.705374, -0.668203),
	vec2(0.977050, -0.108615),
	vec2(0.063326, 0.142369),
	vec2(0.203528, 0.214331),
	vec2(-0.667531, 0.326090),
	vec2(-0.098422, -0.295755),
	vec2(-0.885922, 0.215369),
	vec2(0.566637, 0.605213),
	vec2(0.039766, -0.396100),
	vec2(0.751946, 0.453352),
	vec2(0.078707, -0.715323),
	vec2(-0.075838, -0.529344),
	vec2(0.724479, -0.580798),
	vec2(0.222999, -0.215125),
	vec2(-0.467574, -0.405438),
	vec2(-0.248268, -0.814753),
	vec2(0.354411, -0.887570),
	vec2(0.175817, 0.382366),
	vec2(0.487472, -0.063082),
	vec2(-0.084078, 0.898312),
	vec2(0.488876, -0.783441),
	vec2(0.470016, 0.217933),
	vec2(-0.696890, -0.549791),
	vec2(-0.149693, 0.605762),
	vec2(0.034211, 0.979980),
	vec2(0.503098, -0.308878),
	vec2(-0.016205, -0.872921),
	vec2(0.385784, -0.393902),
	vec2(-0.146886, -0.859249),
	vec2(0.643361, 0.164098),
	vec2(0.634388, -0.049471),
	vec2(-0.688894, 0.007843),
	vec2(0.464034, -0.188818),
	vec2(-0.440840, 0.137486),
	vec2(0.364483, 0.511704),
	vec2(0.034028, 0.325968),
	vec2(0.099094, -0.308023),
	vec2(0.693960, -0.366253),
	vec2(0.678884, -0.204688),
	vec2(0.001801, 0.780328),
	vec2(0.145177, -0.898984),
	vec2(0.062655, -0.611866),
	vec2(0.315226, -0.604297),
	vec2(-0.780145, 0.486251),
	vec2(-0.371868, 0.882138),
	vec2(0.200476, 0.494430),
	vec2(-0.494552, -0.711051),
	vec2(0.612476, 0.705252),
	vec2(-0.578845, -0.768792),
	vec2(-0.772454, -0.090976),
	vec2(0.504440, 0.372295),
	vec2(0.155736, 0.065157),
	vec2(0.391522, 0.849605),
	vec2(-0.620106, -0.328104),
	vec2(0.789239, -0.419965),
	vec2(-0.545396, 0.538133),
	vec2(-0.178564, -0.596057)
);

float texelsize_per_cascade[4] = float[] (
	4.5,
	1.2,
	0.6,
	0.3
);

//#define SKIP_SHADOW_FILTERING

float linstep(float min, float max, float v)
{
	return clamp((v-min) / (max-min), 0, 1);
}

float CalcShadowFactor(int cascade, vec3 normal, vec3 lightdir)
{
	vec4 coords = vert_lightspace_pos[cascade];
	coords = coords / coords.w;
	coords = coords * 0.5 + 0.5;
	
	coords.y /= sunlight.num_cascades;
	coords.y += (float(cascade) / float(sunlight.num_cascades));

	float texelsize = texelsize_per_cascade[cascade] / textureSize(sunlight_shadow_map, 0).y;
	
	float current_depth = coords.z;
	float shadow = 0;
	for (int i = 0; i < 64; ++i)
	{
		shadow += texture(sunlight_shadow_map, vec3(coords.xy + (poissonDisk[i] * texelsize), current_depth));
	}
	return shadow / 64;

//	vec2 moments = texture(sunlight_shadow_map, coords.xy).rg;
	
//	if (distance <= moments.x)
//		return 1.0;
		
//	float variance = moments.y - (moments.x * moments.x);
//	variance = max(variance, 0.000001);
//	float d = distance - moments.x;
//	float shadow_factor = variance / (variance + (d*d));
//	return linstep(0.2, 1, shadow_factor);
}

vec3 CalcDirectionalLights(int cascade, vec3 normal, vec3 view_dir, vec3 mtrl_specular, float mtrl_smoothness)
{
	// First there's the shadowing directional light,

	float shadow_factor = CalcShadowFactor(cascade, normal, sunlight.l.direction.xyz);

	vec3 lighting = CalcBaseLight(	normal,
									sunlight.l.direction.xyz,
									view_dir,
									sunlight.l.diffuse_color.rgb,
									sunlight.l.ambient_color.rgb,
									sunlight.l.diffuse_color.a * mtrl_specular,
									mtrl_smoothness,
									shadow_factor);
	
	// Then we add non-shadowing directional lights.
	for (int i = 0; i < num_dir_lights_used; ++i)
	{
		lighting += CalcBaseLight(	normal,
									dir_lights[i].direction.xyz,
									view_dir,
									dir_lights[i].diffuse_color.rgb,
									dir_lights[i].ambient_color.rgb,
									dir_lights[i].diffuse_color.a * mtrl_specular,
									mtrl_smoothness,
									1);
	}
	
	return lighting;
}

vec3 CalcPointLight(int index, vec3 normal, vec3 view_dir, vec3 mtrl_specular, float mtrl_smoothness)
{
	vec3 light_dir = point_lights[index].position.xyz - vert_world_pos;
	float distance = length(light_dir);
	light_dir /= distance; // normalize direction
	
	vec3 color = CalcBaseLight(	normal,
								light_dir,
								view_dir,
								point_lights[index].color.rgb * point_lights[index].diffuse,
								point_lights[index].color.rgb * point_lights[index].ambient,
								point_lights[index].specular * mtrl_specular,
								mtrl_smoothness,
								1);
	
	float attenuation = 1 - ((distance) / (point_lights[index].position.w)); // position.w is radius
	attenuation = max(0, attenuation);
	
	return color * attenuation * attenuation;
}

vec3 CalcLighting(vec3 normal, vec3 mtrl_specular, float mtrl_smoothness)
{
	// Direction from the eye to the point we're trying to illuminate.
	// For some reason this is backwards...?
	vec3 view_dir = normalize(eye_pos.xyz - vert_world_pos);

	int which_cascade = 0;
	for (which_cascade = 0; which_cascade < sunlight.num_cascades-1; ++which_cascade)
	{
		if (vert_viewspace_z >= sunlight.cascade_boundries[which_cascade].x)
			break;
	}
	
	vec3 light = CalcDirectionalLights(which_cascade, normal, view_dir, mtrl_specular, mtrl_smoothness);
	
	for (int i = 0; i < local_lighting.num_lights; ++i)
	{
		light += CalcPointLight(local_lighting.index[i], normal, view_dir, mtrl_specular, mtrl_smoothness);
	}
	
	return light;
}

#endif // SKIP_LIGHTING

#end // fragment shader
)=="