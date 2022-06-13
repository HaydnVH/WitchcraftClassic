#ifndef HVH_WC_GRAPHICS_SHADOW_H
#define HVH_WC_GRAPHICS_SHADOW_H

#include <stdint.h>
#include <vector>
#include "math/vmath.h"
#include "scene/entity.h"
#include "camera.h"

class Renderer;

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	bool Init(uint32_t resolution, uint32_t num_cascades = 1);
	void clean();

	vmath::mat4 Render(entity::ID light_id);
	vmath::mat4 RenderDirectional(vmath::vec3 direction);
	std::vector<vmath::mat4> RenderDirectional(vmath::vec3 direction, const UniformBufferCamera& frame, float fov, float aspect, const std::vector<float>& cascade_boundries);

	void BindForRead(uint32_t index);

private:
	uint32_t resolution;

#ifdef RENDERER_OPENGL
	uint32_t fbo;
	uint32_t shadow_map;
	uint32_t depth_buffer;
#elif RENDERER_VULKAN
#elif RENDERER_DIRECTX
#endif
};

#endif // HVH_WC_GRAPHICS_SHADOW_H