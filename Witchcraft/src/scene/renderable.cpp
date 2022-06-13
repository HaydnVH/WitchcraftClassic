#include "renderable.h"

#include "graphics/renderer.h"
#include "graphics/model.h"

RenderableComponent::~RenderableComponent()
{
	for (size_t i = 0; i < soa.size(); ++i)
	{
		Model::Unload(soa.get<RCE_MODELFILENAME>(i).c_str());
	}
}

void RenderableComponent::Remove(entity::ID id)
{
	if (hasEntry(id) && soa.get<RCE_MODELPTR>(index(id)))
	{
		Model::Unload(soa.get<RCE_MODELFILENAME>(index(id)).c_str());
	}
	RemoveEntry(id);
}

void RenderableComponent::LoadModel(entity::ID id, const char* filename)
{
	if (hasEntry(id) == false)
		return;

	if (soa.get<RCE_MODELPTR>(index(id)) != nullptr)
	{
		Model::Unload(soa.get<RCE_MODELFILENAME>(index(id)).c_str());
	}

	soa.get<RCE_MODELPTR>(index(id)) = Model::Load(filename);
	soa.get<RCE_MODELFILENAME>(index(id)) = filename;
}

int RenderableComponent::numMeshes(entity::ID id)
{
	if (hasEntry(id) == false || soa.get<RCE_MODELPTR>(index(id)) == nullptr)
		return 0;

	return soa.get<RCE_MODELPTR>(index(id))->numMeshes();
}

void RenderableComponent::ApplyImportTransforms()
{
	for (size_t i = 1; i < soa.size(); ++i)
	{
		entity::ID id = soa.get<RCE_ENTITYID>(i);
		entity::transform::applyPreTransform(id, soa.get<RCE_MODELPTR>(i)->getImportTransform());
	}
}

void RenderableComponent::DrawEverything()
{
	entity::ID* transforms = soa.rawdata<RCE_ENTITYID>();
	Model** models = soa.rawdata<RCE_MODELPTR>();
	for (size_t i = 1; i < soa.size(); ++i)
	{
		if (models[i] != nullptr)
			models[i]->Draw(transforms[i]);
	}
}