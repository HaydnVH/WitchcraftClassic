#ifndef HVH_WC_GRAPHICS_RENDERER_H
#define HVH_WC_GRAPHICS_RENDERER_H

#include "shader.h"
#include "geometry.h"
#include "material.h"
#include "camera.h"
#include "sys/window.h"
#include "scene/scene.h"
#include "math/vmath.h"
#include "skybox.h"

#include "gui/guilayer.h"
class PhysicsDebug;

namespace renderer
{
	/* Performs various OpenGL initialization which does not depend on the windowing platform. */
	/* Must initialize after: Window, Physics. */
	/* Returns false if initialization fails. */
	bool Init();
	void Cleanup();

	void DrawFrame(float interpolation);

	void setCameraEntity(entity::ID id);
	void setZnear(float val);
	void setZfar(float val);
	void setFov(float val);
	void SnapFov();
	void setGuiLayer(GuiLayer* newguilayer);
	void LoadSkybox(const char* name);

	void InitLua();

	void SubmitRenderable(entity::ID transformid, Geometry* geom, Material* mat, int start = 0, int count = 0);

	void DrawShadows(const UniformBufferCamera& cam);


	struct RenderQueueEntry
	{
		entity::ID transformid;
		Geometry* geom;
		Material* mat;
		int start, count;
	};

	struct Pipeline
	{
		Pipeline() :shader(nullptr) {}

		Shader* shader;
		std::vector<RenderQueueEntry> queue;
	};

	struct PipelineCollection
	{
		Pipeline opaque_static;
		Pipeline opaque_skinned;
		Pipeline transparent_static;
		Pipeline transparent_skinned;
		Pipeline shadow_static;
		Pipeline shadow_skinned;

		void ClearAll()
		{
			opaque_static.queue.clear();
			opaque_skinned.queue.clear();
			transparent_static.queue.clear();
			transparent_skinned.queue.clear();
			shadow_static.queue.clear();
			shadow_skinned.queue.clear();
		}
	};

	void DrawQueue(Pipeline& pipeline, const UniformBufferCamera& cam);
};


#endif // HVH_WC_GRAPHICS_RENDERERGL_H