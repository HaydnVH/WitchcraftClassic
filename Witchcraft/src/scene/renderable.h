#ifndef HVH_WC_SCENE_RENDERABLE_H
#define HVH_WC_SCENE_RENDERABLE_H

#include "component.h"
#include "transform.h"

class Renderer;
class Model;

enum RenderableComponentEnum
{
	RCE_ENTITYID = 0,
	RCE_MODELPTR,
	RCE_MODELFILENAME
};

class RenderableComponent : protected ComponentTable<Model*, std::string>
{
public:
	RenderableComponent()
	: ComponentTable<Model*, std::string>(nullptr, "")
	{}
	~RenderableComponent();

	using ComponentTable<Model*, std::string>::hasEntry;

	void AddEmpty(entity::ID id)
	{
		if (entity::transform::has(id) == false)
		{
			plog::error("Cannot add renderable component to an entity which lacks a transform component.\n");
			return;
		}

		AddEntry(id, nullptr, "");
	}

	void Remove(entity::ID id);
	void LoadModel(entity::ID id, const char* filename);
	int numMeshes(entity::ID id);

	Model* getModelPtr(entity::ID id)
		{ return soa.get<RCE_MODELPTR>(index(id)); }

	void ApplyImportTransforms();
	void DrawEverything();

	void InitLua();
};

#endif // HVH_WC_SCENE_RENDERABLE_H