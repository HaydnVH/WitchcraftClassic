#ifndef HVH_WC_GRAPHICS_CAMERA_H
#define HVH_WC_GRAPHICS_CAMERA_H

#include "math/vmath.h"

struct UniformBufferCamera
{
	vmath::mat4 view;
	vmath::mat4 proj;
	vmath::mat4 projview;
	vmath::mat4 invproj;
	vmath::vec4 eye_pos;
	vmath::vec4 eye_dir;
	vmath::vec2 screen_size;
	float aspect_ratio;
	float frame_interpolation;
	uint32_t logical_frame_counter;
	uint32_t display_frame_counter;
};

#endif // HVH_WC_GRAPHICS_CAMERA_H