#ifdef RENDERER_OPENGL
#include "shadow.h"

#include "renderer.h"
#include <GL/glew.h>
#include <GL/GL.h>
//#include "ext_gl/extensions.h"

#include "scene/transform.h"
#include "scene/renderable.h"
#include "graphics/camera.h"
#include "scene/entity.h"

#include "renderer.h"

#include "sys/printlog.h"

using namespace std;
using namespace vmath;

ShadowMap::ShadowMap()
:	fbo(0),
	shadow_map(0),
	depth_buffer(0)
{}

ShadowMap::~ShadowMap()
{}

bool ShadowMap::Init(uint32_t resolution, uint32_t num_cascades)
{
	glGenTextures(1, &shadow_map);
	glBindTexture(GL_TEXTURE_2D, shadow_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, resolution, resolution * num_cascades, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

//	glGenRenderbuffers(1, &depth_buffer);
//	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution, resolution * num_cascades);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map, 0);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		plog::error("Failed to create shadow framebuffer.\n");
		plog::errmore("Status: %i\n", status);
		return false;
	}

	this->resolution = resolution;

	return true;
}

void ShadowMap::clean()
{
	glDeleteTextures(1, &shadow_map);
//	glDeleteRenderbuffers(1, &depth_buffer);
	glDeleteFramebuffers(1, &fbo);
}
/*
mat4 ShadowMap::Render(Renderer& renderer, entity::ID id)
{
	if (component::transform::exists(id) == false)
	{
		plog::error(ERRMARK, "Cannot draw shadow map with entity '", id, "' because it does not have a transform.\n");
		return MAT4_IDENTITY;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, resolution, resolution);

	main_camera::UniformBufferCamera this_frame;

	this_frame.view = mat4::translation(-component::transform::get_position(id));
	this_frame.view *= mat4::rotation(-component::transform::get_rotation(id));

	this_frame.proj = mat4::ortho(-10, 10, -10, 10, -10, 10);
	this_frame.projview = this_frame.proj * this_frame.view;

	this_frame.inv_proj = this_frame.proj.inverted();
	this_frame.eye_pos = vec4(component::transform::get_position(id), 0.0f);
	this_frame.screen_size = { (float)resolution, (float)resolution };
//	glEnable(GL_MULTISAMPLE);

	renderqueue::Draw(RENDER_QUEUE_SHADOW, this_frame);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return this_frame.projview;
}
*/

mat4 ShadowMap::RenderDirectional(vec3 direction)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, resolution, resolution);

	UniformBufferCamera shcam;

	shcam.view = mat4::look_at(VEC3_ZERO, direction, VEC3_UP);

	shcam.proj = mat4::ortho(-5, 5, -5, 5, -5, 5);
	shcam.projview = shcam.proj * shcam.view;
	shcam.invproj = shcam.proj.inverted();
	shcam.eye_pos = { 0,0,0,0 };
	shcam.screen_size = { (float)resolution, (float)resolution };

	renderer::DrawShadows(shcam);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return shcam.projview;
}

vector<mat4> ShadowMap::RenderDirectional(vmath::vec3 direction, const UniformBufferCamera& frame, float fov, float aspect, const vector<float>& cascade_boundries)
{
	vector<mat4> result;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UniformBufferCamera shcam;

	shcam.view = mat4::look_at(VEC3_ZERO, -direction, VEC3_UP);
	shcam.eye_pos = { 0,0,0,0 };
	shcam.screen_size = { (float)resolution, (float)resolution };
	shcam.frame_interpolation = frame.frame_interpolation;

	// We need to create a bounding volume for our projection matrix.
	// What data do we need for this?
	// We need to create a custom camera matrix using the cascade slices.
	// We need the inverse of this matrix, to transform corners from camera space to view space
	// We need the inverse of the view matrix, to transform corners from view space to world space
	// We need our own view matrix, to transform corners from world space to light space
	// Then we can use min/max to determine the bounds of our own orthographic matrix.

	uint32_t num_cascades = (uint32_t)cascade_boundries.size() + 1;
	if (num_cascades > 5)
	{
		plog::error("Shader cannot handle more than 5 cascades!\n");
		num_cascades = 5;
	}

	static vector<float> saved_longest_diagonals;
	static float saved_aspect_ratio = 0.0f;

	if (saved_longest_diagonals.size() == 0)
	{
		saved_longest_diagonals.resize(num_cascades);
	}
		

	for (uint32_t cascade = 0; cascade < num_cascades; ++cascade)
	{
		// This is where we'll be rendering the shadows for this cascade to.
		glViewport(0, resolution * cascade, resolution, resolution);

		// Figure out the near and far planes.
		float near_plane;
		float far_plane;

		if (cascade == 0)
			near_plane = 0.01f;// main_camera::get_znear();
		else
			near_plane = cascade_boundries[cascade - 1];

		if (cascade == cascade_boundries.size())
			far_plane = 1000.0f;// main_camera::get_zfar();
		else
			far_plane = cascade_boundries[cascade];

		// Create a projection matrix for this cascade.
		// This will transform the corner from camera space to view space.
		mat4 proj = mat4::perspective(fov, aspect, near_plane, far_plane);

		// B.inverted()*A.inverted() == C.inverted() == (A*B).inverted()
		// This is now an inverted viewproj matrix, which transforms the corner from camera space to world space.
		mat4 inv_viewproj = (proj * frame.view).inverted();

		// This is the matrix that transforms each corner from camera space to light space.
		mat4 matrix = shcam.view * inv_viewproj;

		vec3 min_ = { FLT_MAX, FLT_MAX, FLT_MAX };
		vec3 max_ = -min_;

		vec4 corners[8] =
		{
			// Far plane
			{ 1, 1, 1, 1 },
			{ -1, 1, 1, 1 },
			{ 1, -1, 1, 1 },
			{ -1, -1, 1, 1 },
			// Near plane
			{ 1, 1, -1, 1 },
			{ -1, 1, -1, 1 },
			{ 1, -1, -1, 1 },
			{ -1, -1, -1, 1 }
		};

		// For each frustum corner...
		for (uint32_t i = 0; i < 8; ++i)
		{
			corners[i] = matrix * corners[i];
			corners[i] /= corners[i].w;
			// Transform each corner from camera space to view space using the inverse projection matrix,
			// Then from view space to world space using the inverse view matrix,
			// Then from world space to light space using this light's view matrix.

			min_.x = fminf(min_.x, corners[i].x);
			min_.y = fminf(min_.y, corners[i].y);
			min_.z = fminf(min_.z, corners[i].z);

			max_.x = fmaxf(max_.x, corners[i].x);
			max_.y = fmaxf(max_.y, corners[i].y);
			max_.z = fmaxf(max_.z, corners[i].z);
		}

		// We only want to calculate the 'longest diagonal' once for each cascade,
		// and then only update it if the screen's aspect ratio changes.
	//	if (aspect != saved_aspect_ratio)
	//	{
			// Get the longest diagonal between corners of the transformed frustum
			float diagonal_nearfar = vec3::distance(corners[0].xyz, corners[7].xyz);
			float diagonal_farfar = vec3::distance(corners[0].xyz, corners[3].xyz);
			saved_longest_diagonals[cascade] = fmaxf(diagonal_nearfar, diagonal_farfar);
	//	}

		vec2 border_offset = (vec2(saved_longest_diagonals[cascade], saved_longest_diagonals[cascade]) - (max_.xy - min_.xy)) * 0.5f;
		max_.xy += border_offset;
		min_.xy -= border_offset;

		// Round the bounding box to the nearest texel unit to reduce flickering.
		vec2 units_per_texel = 2.0f * (max_.xy - min_.xy) / (float)resolution;
		min_.x = floor(min_.x / units_per_texel.x)*units_per_texel.x;
		max_.x = floor(max_.x / units_per_texel.x)*units_per_texel.x;
		min_.y = floor(min_.y / units_per_texel.y)*units_per_texel.y;
		max_.y = floor(max_.y / units_per_texel.y)*units_per_texel.y;

		// The "+50" is here to account for objects which are beyond the intersection
		// of the camera's frustum and the light's frustum, but still need to cast a shadow.
		// Ideally, this should be obtained by testing against the AABBs of the scene objects.
		shcam.proj = mat4::ortho(min_.x, max_.x, min_.y, max_.y, -(max_.z + 50), -min_.z);
		shcam.projview = shcam.proj * shcam.view;
		shcam.invproj = MAT4_IDENTITY;

		renderer::DrawShadows(shcam);

		result.push_back(shcam.projview);
	}

	saved_aspect_ratio = aspect;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return result;
}

void ShadowMap::BindForRead(uint32_t index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, shadow_map);
}

#endif // RENDERER_OPENGL