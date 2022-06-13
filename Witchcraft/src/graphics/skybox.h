#ifndef HVH_WC_GRAPHICS_SKYBOX_H
#define HVH_WC_GRAPHICS_SKYBOX_H

#include <vector>
#include "math/vmath.h"
#include "texture.h"
#include "geometry.h"
#include "shader.h"
#include "camera.h"
#include "shadow.h"

class Renderer;
class ShaderManager;

constexpr const char* SKYBOX_TXT_FILE_EXT = ".sky.xml";
constexpr const char* SKYBOX_BIN_FILE_EXT = ".sky.bin";

class Skybox
{
public:
	Skybox()
	:	light_dir(vmath::VEC3_DOWN),
		light_col(vmath::VEC3_ONE),
		ambient_col(vmath::VEC3_ZERO),
		geom(), tex(),
		shader(nullptr),
		light_buffer_id(0)
	{}

	void Load(const char* name);
	void Draw(const UniformBufferCamera& cam);
	void Lighting(const UniformBufferCamera& cam, float fov);

private:
	vmath::vec3 light_dir;
	vmath::vec3 light_col;
	vmath::vec3 ambient_col;

	Geometry geom;
	Texture tex;
	Shader* shader;
	uint32_t light_buffer_id;

	ShadowMap shadow;
	std::vector<float> cascades;

	enum SkyboxShaderUniforms
	{
		UNIFORM_MATRIX_MVP = 0,
		UNIFORM_MATRIX_TRANSFORM,
		UNIFORM_TEXTURE_SKYBOX,
	};

	const std::vector<const char*> SKYBOX_SHADER_UNIFORM_NAMES
	{
		"mvp",
		"transform",
		"skybox_texture"
	};
};

#endif // HVH_WC_GRAPHICS_SKYBOX_H