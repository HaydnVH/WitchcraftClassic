#ifndef HVH_WC_GRAPHICS_MATERIAL_H
#define HVH_WC_GRAPHICS_MATERIAL_H

#include "shader.h"
#include "texture.h"
#include "math/vmath.h"

constexpr const char* MATERIAL_PATH = "materials/";
constexpr const char* MATERIAL_EXTENSION = ".mat.xml";

enum MaterialEnum
{
	MATERIAL_FLAG_TRANSPARENT = 1 << 0,
	MATERIAL_FLAG_CASTSHADOWS = 1 << 1,
	MATERIAL_FLAG_SKINNED = 1 << 2,
};

enum MaterialShaderUniforms
{
	UNIFORM_MATRIX_MVP = 0,
	UNIFORM_MATRIX_TRANSFORM,
	UNIFORM_MATERIAL_COLOR,
	UNIFORM_MATERIAL_SMOOTHNESS,
	UNIFORM_MATERIAL_SPECULAR,
	UNIFORM_MATERIAL_DIFFUSE_TEXTURE,
	UNIFORM_MATERIAL_NORMAL_TEXTURE,
	UNIFORM_MATERIAL_SPECULAR_TEXTURE,
	UNIFORM_MATERIAL_GLOW_TEXTURE,
	UNIFORM_RANDOM_TEXTURE,
	UNIFORM_SUNLIGHT_SHADOWMAP,
	UNIFORM_NUM_POINT_LIGHTS,
	UNIFORM_LIGHT_NUM_LOCAL_LIGHTS,
	UNIFORM_LIGHT_LOCAL_INDEX_0,
	UNIFORM_LIGHT_LOCAL_INDEX_1,
	UNIFORM_LIGHT_LOCAL_INDEX_2,
	UNIFORM_LIGHT_LOCAL_INDEX_3,
	UNIFORM_LIGHT_LOCAL_INDEX_4,
	UNIFORM_LIGHT_LOCAL_INDEX_5,
	UNIFORM_LIGHT_LOCAL_INDEX_6,
	UNIFORM_LIGHT_LOCAL_INDEX_7,
	UNIFORM_LIGHT_LOCAL_INDEX_8,
	UNIFORM_LIGHT_LOCAL_INDEX_9,
	UNIFORM_LIGHT_LOCAL_INDEX_10,
	UNIFORM_LIGHT_LOCAL_INDEX_11,
	UNIFORM_LIGHT_LOCAL_INDEX_12,
	UNIFORM_LIGHT_LOCAL_INDEX_13,
	UNIFORM_LIGHT_LOCAL_INDEX_14,
	UNIFORM_LIGHT_LOCAL_INDEX_15,
};

const std::vector<const char*> MATERIAL_SHADER_UNIFORM_NAMES =
{
	"mvp",
	"transform",
	"material.color",
	"material.smoothness",
	"material.specular",
	"material.diffuse_texture",
	"material.normal_texture",
	"material.specular_texture",
	"material.glow_texture",
	"random_texture",
	"sunlight_shadow_map",
	"num_point_lights",
	"local_lighting.num_lights",
	"local_lighting.index[0]",
	"local_lighting.index[1]",
	"local_lighting.index[2]",
	"local_lighting.index[3]",
	"local_lighting.index[4]",
	"local_lighting.index[5]",
	"local_lighting.index[6]",
	"local_lighting.index[7]",
	"local_lighting.index[8]",
	"local_lighting.index[9]",
	"local_lighting.index[10]",
	"local_lighting.index[11]",
	"local_lighting.index[12]",
	"local_lighting.index[13]",
	"local_lighting.index[14]",
	"local_lighting.index[15]",
};

class Material
{
public:
	Material()
	:	//shader_ptr(nullptr),
		//skinned_shader_ptr(nullptr),
		color({1, 1, 1, 1}),
		spec_smooth(128),
		spec_amount(0),
		mtrl_flags(0),
		refcount_(0)
	{}

	// A material contains a shader, and a collection of textures and settings to feed into that shader.
	// The shader is just a reference, so multiple materials can share a material
	// Textures are truly contained within the material, but standalone textures also exist conceptually.

	static void initShaders();

	static Material* Load(const char* name);
	static void Unload(const char* name);

	bool use(Shader* shader);

//	void shader(Shader* new_shader) { shader_ptr = new_shader; }
//	Shader* shader() { return shader_ptr; }
//	Shader* skinned_shader() { return skinned_shader_ptr; }

	uint32_t flags()
	{
		return mtrl_flags;
	}

	void increment_refcount()
	{
		refcount_++;
	}

private:
//	std::string shader_name;
//	Shader* shader_ptr;

//	std::string skinned_shader_name;
//	Shader* skinned_shader_ptr;

	vmath::vec4 color;
	float spec_smooth;
	float spec_amount;
	uint32_t mtrl_flags;

	Texture diffuse_texture;
	Texture normal_texture;
	Texture specular_texture;
	Texture glow_texture;

	uint32_t refcount_;
};

#endif