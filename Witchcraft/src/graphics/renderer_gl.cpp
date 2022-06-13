#ifdef RENDERER_OPENGL
#include "renderer.h"
#include <GL/glew.h>
#include <GL/GL.h>

#include "appconfig.h"
#include "sys/printlog.h"
#include "sys/window.h"

#include "camera.h"
#include "animation_controller.h"
using namespace vmath;

#include "physics/physics_system.h"

namespace renderer {
namespace {

	int screen_width = 0, screen_height = 0;
	uint32_t camera_buffer_loc = 0;
	entity::ID camera_entity = 0;

	Skybox* skybox = nullptr;

	float znear = 0.01f, zfar = 1000.0f;
	float fov_prev = 85.0f, fov_now = 85.0f;

	Shader* debugshader = nullptr;

	GuiLayer* guilayer = nullptr;

	PipelineCollection pipelines;

} // namespace <anon>

void debug_callback(GLenum src, GLenum type, unsigned int id, GLenum severity, int length, const char* msg, const void* uparam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		//					plog::info("[GL] [", src, "] [", type, "] [INFO]\n", INFOMORE, msg, '\n');
		break;
	case GL_DEBUG_SEVERITY_HIGH:
//		plog::error("[GL] [%s] [%s] [HIGH]\n", src, type);
		plog::error("%s\n", msg);
		break;
	case GL_DEBUG_SEVERITY_LOW:
	case GL_DEBUG_SEVERITY_MEDIUM:
//		plog::warning("[GL] [%s] [%s] [WARN]\n", src, type);
		plog::warning("%s\n", msg);
		break;
	default: break;
	}
}

bool Init()
{
	// Enforce dependancies
	if (!window::isOpen())
	{
		plog::fatal("Programmer Error: Window must initialize before Renderer!");
		return false;
	}

	// Enable debug output from GL_ARB_debug_output
	if (app::use_debug)
	{
		glEnable(GL_DEBUG_OUTPUT);

		if (glDebugMessageCallback)
		{
			glDebugMessageCallback(debug_callback, nullptr);
			plog::info("OpenGL debug output enabled.\n");
		}
		else
		{
			plog::warning("OpenGL debug output not available.\n");
		}
	}

	Material::initShaders();

	// Initalize OpenGL state
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LESS); <- default
	glEnable(GL_CULL_FACE);
//	glCullFace(GL_BACK); <- default
//	glFrontFace(GL_CCW); <- default
	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_MULTISAMPLE);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// This offset is for shadow mapping.  We only need to specify it once, so we do it here.
	glPolygonOffset(5.0f, 10.0f);

	// Vertex attribute #1 is for per-vertex colors.
	// Since many models wont include vertex formats, we want the default to be white.
	// This lets us multiply the vertex color with the material color,
	// so shaders don't need to care about whether or not vertex colors exist in the first place.
	glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

	Shader::InitDefaultShaders();
	skybox = new Skybox();

	camera_buffer_loc = Shader::RegisterUniformBuffer("CAMERA");

	// Get pipeline shaders and find their uniform locations.
	pipelines.opaque_static.shader = Shader::getShader("standard_material");
	pipelines.opaque_static.shader->FindUniformLocations(MATERIAL_SHADER_UNIFORM_NAMES);
	pipelines.opaque_skinned.shader = Shader::getShader("standard_material|SKELETAL_ANIMATION");
	pipelines.opaque_skinned.shader->FindUniformLocations(MATERIAL_SHADER_UNIFORM_NAMES);

	pipelines.transparent_static.shader = Shader::getShader("standard_material");
	pipelines.transparent_skinned.shader = Shader::getShader("standard_material|SKELETAL_ANIMATION");
	
	pipelines.shadow_static.shader = Shader::getShader("shadow");
	pipelines.shadow_static.shader->FindUniformLocations({"mvp"});
	pipelines.shadow_skinned.shader = Shader::getShader("shadow|SKELETAL_ANIMATION");
	pipelines.shadow_skinned.shader->FindUniformLocations({"mvp"});

	debugshader = Shader::getShader("debugdraw");
	debugshader->FindUniformLocations({"transparency"});

	AnimationController::initShaders();

	InitLua();

	return true;
}

void Cleanup()
{
	Shader::Cleanup();
} 

void DrawFrame(float interpolation)
{
	// Calculate transform matrices.
	entity::transform::CalcMatrices(interpolation);
	scene::getRenderableComponent().ApplyImportTransforms();
	scene::getAnimatorComponent().DisplayUpdate(interpolation);

	// Get the size of the window and account for it changing.
	int w, h;
	window::getDrawableSize(w, h);
	if (h == 0) h = 1;
	if (w != screen_width || h != screen_height)
	{
//		glViewport(0, 0, w, h);
		screen_width = w;
		screen_height = h;
	}

	UniformBufferCamera camera = {};
	camera.screen_size = { (float)screen_width, (float)screen_height };
	camera.aspect_ratio = (float)screen_width / (float)screen_height;
	camera.frame_interpolation = interpolation;

	if (entity::transform::has(camera_entity))
	{
		camera.view = mat4::rotation(-entity::transform::interpolateRot(camera_entity, interpolation) * quat::euler(TO_RADIANS*-90, 0, 0)) *
					  mat4::translation(-entity::transform::interpolatePos(camera_entity, interpolation));

		camera.eye_pos = vec4(entity::transform::interpolatePos(camera_entity, interpolation), 0.0f);
	}
	else
	{
		camera.view = mat4::rotation(quat::euler(TO_RADIANS*-90, 0, 0)) * mat4::translation(0, 0, 0);

		camera.eye_pos = { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	camera.eye_dir = camera.view * vec4(VEC3_FORWARD, 0.0f);
	camera.proj = mat4::perspective(lerp(fov_prev, fov_now, interpolation), camera.aspect_ratio, znear, zfar);
	camera.projview = camera.proj * camera.view;
	camera.invproj = camera.proj.inverted();

	Shader::UploadUniformBuffer(camera_buffer_loc, sizeof(UniformBufferCamera), &camera);

	// Clear the frmebuffer so we can draw to it.
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Clear the rendering lists
	pipelines.ClearAll();

	// Populate the rendering lists
	scene::getRenderableComponent().DrawEverything();

	// Do directional lighting
//	glCullFace(GL_FRONT);
	glEnable(GL_POLYGON_OFFSET_FILL);
//	glDisable(GL_CULL_FACE);
	skybox->Lighting(camera, lerp(fov_prev, fov_now, interpolation));
//	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
//	glCullFace(GL_BACK);

	// Correct viewport state which was changed by drawing shadows.
	glViewport(0, 0, screen_width, screen_height);

	// Draw the rendering lists
	DrawQueue(pipelines.opaque_static, camera);
	DrawQueue(pipelines.opaque_skinned, camera);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	DrawQueue(pipelines.transparent_static, camera);
	DrawQueue(pipelines.transparent_skinned, camera);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	// Draw the skybox
	glDepthFunc(GL_LEQUAL);
	skybox->Draw(camera);
	glDepthFunc(GL_LESS);

	// Draw debug stuff.
	// This one line is the whole reason why Renderer depends on Physics.
	physics::getDebug().Draw(debugshader, interpolation);

	// Draw the gui layer.
	if (guilayer != nullptr)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);

		guilayer->Draw(camera);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	window::Display();
}

void setCameraEntity(entity::ID id) { camera_entity = id; }
void setZnear(float val) { znear = val; }
void setZfar(float val) { zfar = val; }
void setFov(float val) { fov_now = val; }
void SnapFov() { fov_prev = fov_now; }

void setGuiLayer(GuiLayer* newguilayer)
{
	guilayer = newguilayer;
	guilayer->initShaders();
}

void LoadSkybox(const char* name) { skybox->Load(name); }

void SubmitRenderable(entity::ID transformid, Geometry* geom, Material* mat, int start, int count)
{
	if (!entity::transform::has(transformid) || !geom || !mat)
		{ plog::error("Objects require a valid transform id, geometry, and material to be drawn.\n"); return; }

	if (mat->flags() & MATERIAL_FLAG_TRANSPARENT)
	{
		if (scene::getAnimatorComponent().hasEntry(transformid))
			{ pipelines.transparent_skinned.queue.push_back({ transformid, geom, mat, start, count }); }
		else
			{ pipelines.transparent_static.queue.push_back({ transformid, geom, mat, start, count }); }
	}
	else
	{
		if (scene::getAnimatorComponent().hasEntry(transformid))
		{
			pipelines.opaque_skinned.queue.push_back({ transformid, geom, mat, start, count });
			pipelines.shadow_skinned.queue.push_back({ transformid, geom, mat, start, count });
		}
		else
		{
			pipelines.opaque_static.queue.push_back({ transformid, geom, mat, start, count });
			pipelines.shadow_static.queue.push_back({ transformid, geom, mat, start, count });
		}
	}
}

void DrawShadows(const UniformBufferCamera& cam)
{
	if (!pipelines.shadow_static.shader || !pipelines.shadow_static.shader->use())
		return;

	for (size_t i = 0; i < pipelines.shadow_static.queue.size(); ++i)
	{
		RenderQueueEntry& item = pipelines.shadow_static.queue[i];

		mat4 matrix = entity::transform::getMatrix(item.transformid);
		mat4 mvp = cam.projview * matrix;

		pipelines.shadow_static.shader->setUniform(0, mvp);

		// Finally draw the item
		item.geom->Draw(item.start, item.count);
	}

	if (!pipelines.shadow_skinned.shader || !pipelines.shadow_skinned.shader->use())
		return;

	for (size_t i = 0; i < pipelines.shadow_skinned.queue.size(); ++i)
	{
		RenderQueueEntry& item = pipelines.shadow_skinned.queue[i];

		mat4 matrix = entity::transform::getMatrix(item.transformid);
		mat4 mvp = cam.projview * matrix;

		scene::getAnimatorComponent().UploadSkeleton(item.transformid);
		
		pipelines.shadow_skinned.shader->setUniform(0, mvp);

		item.geom->Draw(item.start, item.count);
	}
}

void DrawQueue(Pipeline& pipeline, const UniformBufferCamera& cam)
{
	if (!pipeline.shader || !pipeline.shader->use())
		return;

	for (size_t i = 0; i < pipeline.queue.size(); ++i)
	{
		RenderQueueEntry& item = pipeline.queue[i];
		item.mat->use(pipeline.shader);

		// TODO: Split RenderQueue into two functions so we don't need to branch here.
		if (scene::getAnimatorComponent().hasEntry(item.transformid))
			scene::getAnimatorComponent().UploadSkeleton(item.transformid);

		// TODO: Set up point lights for this object.

		mat4 matrix = entity::transform::getMatrix(item.transformid);
		mat4 mvp = cam.projview * matrix;

		pipeline.shader->setUniform(UNIFORM_MATRIX_TRANSFORM, matrix);
		pipeline.shader->setUniform(UNIFORM_MATRIX_MVP, mvp);

		// Finally draw the item.
		item.geom->Draw(item.start, item.count);
	}
}

} // namespace renderer

#endif // RENDERER_OPENGL