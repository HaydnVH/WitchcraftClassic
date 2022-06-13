#include "material.h"

#include <map>
using namespace std;

#include "sys/printlog.h"
#include "filesystem/file_manager.h"
#include "tools/stringhelper.h"
#include "tools/fixedstring.h"
#include "tools/xmlhelper.h"
#include "tools/colors.h"
using namespace pugi;

namespace {

	map<string, Material*> loaded_materials;

} // namespace <anon>

void Material::initShaders()
{}

Material* Material::Load(const char* name)
{
	Material* result = NULL;

	if (loaded_materials.count(name))
	{
		result = loaded_materials[name];
		++result->refcount_;
		return result;
	}

	char full_filename[64];
	snprintf(full_filename, 64, "%s%s%s", MATERIAL_PATH, name, MATERIAL_EXTENSION);
	
	InFile file = filemanager::LoadSingleFile(full_filename);
	if (!file.is_open())
	{
		plog::error("Could not open '%s'.\n", full_filename);
		return NULL;
	}

	string contents = file.contents();

	xml_document doc;
	xml_node root;

	xml_parse_result parse_result = doc.load_buffer_inplace(&contents[0], contents.size());
	if (!parse_result || !(root = doc.child("material")))
	{
		plog::error("Failed to parse %s%s%s", MATERIAL_PATH, name, MATERIAL_EXTENSION);
		plog::errmore("Description: %s", parse_result.description());
		plog::errmore("Offset: %s\n", parse_result.offset);
		plog::errmore(" (error at [...%s]\n", &contents[parse_result.offset]);
		return NULL;
	}

	result = new Material;

	xml_node node;
	xml_node subnode;
	xml_node leaf;

	// Textures are the most important part, so let's load them first.
	xml_node textures_node;
	if (textures_node = root.child("textures"))
	{
		// Diffuse texture
		if (node = textures_node.child("diffuse"))
		{
			// Try to load the image before we move on
			const char* filename = readXML(node, "image", (const char*)NULL);
			if (filename && (result->diffuse_texture.LoadImage(filename, true) != 0))
			{
				// Set filtering settings
				if (subnode = node.child("filtering"))
				{
					TexEnum minfilter = TEXFILTER_LINEAR;
					TexEnum magfilter = TEXFILTER_LINEAR;
					float aniso = 0.0f;

					if (leaf = subnode.child("min"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							minfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("mag"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							magfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("aniso"))
					{
						sscanf(leaf.text().as_string(), "%f", &aniso);
					}

					result->diffuse_texture.setFiltering(minfilter, magfilter, aniso);
				}

				// Set wrapping settings
				if (subnode = node.child("wrapping"))
				{
					TexEnum s = TEXWRAP_REPEAT;
					TexEnum t = TEXWRAP_REPEAT;

					if (leaf = subnode.child("s"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							s = TEXWRAP_CLAMP;
						}
					}

					if (leaf = subnode.child("t"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							t = TEXWRAP_CLAMP;
						}
					}

					result->diffuse_texture.setWrapping(s, t);
				}

				// Generate mipmaps, if requested.
				if (node.child("generate_mipmaps"))
				{
					result->diffuse_texture.GenerateMipmaps();
				}
			}
		}

		// Normal texture
		if (node = textures_node.child("normal"))
		{
			// Try to load the image before we move on
			const char* filename = readXML(node, "image", (const char*)NULL);
			if (filename && (result->normal_texture.LoadImage(filename, false) != 0))
			{
				// Set filtering settings
				if (subnode = node.child("filtering"))
				{
					TexEnum minfilter = TEXFILTER_LINEAR;
					TexEnum magfilter = TEXFILTER_LINEAR;
					float aniso = 0.0f;

					if (leaf = subnode.child("min"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							minfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("mag"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							magfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("aniso"))
					{
						sscanf(leaf.text().as_string(), "%f", &aniso);
					}

					result->normal_texture.setFiltering(minfilter, magfilter, aniso);
				}

				// Set wrapping settings
				if (subnode = node.child("wrapping"))
				{
					TexEnum s = TEXWRAP_REPEAT;
					TexEnum t = TEXWRAP_REPEAT;

					if (leaf = subnode.child("s"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							s = TEXWRAP_CLAMP;
						}
					}

					if (leaf = subnode.child("t"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							t = TEXWRAP_CLAMP;
						}
					}

					result->normal_texture.setWrapping(s, t);
				}

				// Generate mipmaps, if requested.
				if (node.child("generate_mipmaps"))
				{
					result->normal_texture.GenerateMipmaps();
				}
			}
		}

		// Specular texture
		if (node = textures_node.child("specular"))
		{
			// Try to load the image before we move on
			const char* filename = readXML(node, "image", (const char*)NULL);
			if (filename && (result->specular_texture.LoadImage(filename, true) != 0))
			{
				// Set filtering settings
				if (subnode = node.child("filtering"))
				{
					TexEnum minfilter = TEXFILTER_LINEAR;
					TexEnum magfilter = TEXFILTER_LINEAR;
					float aniso = 0.0f;

					if (leaf = subnode.child("min"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							minfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("mag"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							magfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("aniso"))
					{
						sscanf(leaf.text().as_string(), "%f", &aniso);
					}

					result->specular_texture.setFiltering(minfilter, magfilter, aniso);
				}

				// Set wrapping settings
				if (subnode = node.child("wrapping"))
				{
					TexEnum s = TEXWRAP_REPEAT;
					TexEnum t = TEXWRAP_REPEAT;

					if (leaf = subnode.child("s"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							s = TEXWRAP_CLAMP;
						}
					}

					if (leaf = subnode.child("t"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							t = TEXWRAP_CLAMP;
						}
					}

					result->specular_texture.setWrapping(s, t);
				}

				// Generate mipmaps, if requested.
				if (node.child("generate_mipmaps"))
				{
					result->specular_texture.GenerateMipmaps();
				}
			}
		}

		// Glow texture
		if (node = textures_node.child("glow"))
		{
			// Try to load the image before we move on
			const char* filename = readXML(node, "image", (const char*)NULL);
			if (filename && (result->glow_texture.LoadImage(filename, true) != 0))
			{
				// Set filtering settings
				if (subnode = node.child("filtering"))
				{
					TexEnum minfilter = TEXFILTER_LINEAR;
					TexEnum magfilter = TEXFILTER_LINEAR;
					float aniso = 0.0f;

					if (leaf = subnode.child("min"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							minfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("mag"))
					{
						if (strcmp(leaf.text().as_string(), "nearest") == 0)
						{
							magfilter = TEXFILTER_NEAREST;
						}
					}

					if (leaf = subnode.child("aniso"))
					{
						sscanf(leaf.text().as_string(), "%f", &aniso);
					}

					result->glow_texture.setFiltering(minfilter, magfilter, aniso);
				}

				// Set wrapping settings
				if (subnode = node.child("wrapping"))
				{
					TexEnum s = TEXWRAP_REPEAT;
					TexEnum t = TEXWRAP_REPEAT;

					if (leaf = subnode.child("s"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							s = TEXWRAP_CLAMP;
						}
					}

					if (leaf = subnode.child("t"))
					{
						if (strcmp(leaf.text().as_string(), "clamp") == 0)
						{
							t = TEXWRAP_CLAMP;
						}
					}

					result->glow_texture.setWrapping(s, t);
				}

				// Generate mipmaps, if requested.
				if (node.child("generate_mipmaps"))
				{
					result->glow_texture.GenerateMipmaps();
				}
			}
		}
	}

	// Get the material color.
	if (node = root.child("color"))
	{
		const char* str = node.text().as_string();
		if (colors::get(str, result->color) == false)
			result->color = { 1, 1, 1, 1 };
	}

	if (node = root.child("specular"))
	{
		if (subnode = node.child("smoothness"))
		{
			sscanf(subnode.text().as_string(), "%f", &result->spec_smooth);
		}

		if (subnode = node.child("intensity"))
		{
			sscanf(subnode.text().as_string(), "%f", &result->spec_amount);
		}
	}

	// Get any special shaders that the material relies on.
//	if (node = root.child("shader"))
//	{
//		result->shader_ptr = sm->getShader(node.text().as_string());
//		result->shader_name = node.text().as_string();
//	}
	
	// Get any flags
	if (node = root.child("flags"))
	{
		if (node.child("transparent"))
			result->mtrl_flags |= MATERIAL_FLAG_TRANSPARENT;

		if (node.child("casts_shadows"))
			result->mtrl_flags |= MATERIAL_FLAG_CASTSHADOWS;
	}

	result->refcount_ = 1;
	loaded_materials[name] = result;
	return result;
}

void Material::Unload(const char* name)
{
	if (loaded_materials.count(name))
	{
		Material* result = loaded_materials[name];
		--result->refcount_;
		if (result->refcount_ <= 0)
		{
			delete result;
			loaded_materials.erase(name);
		}
	}
}

bool Material::use(Shader* shader)
{
	shader->setUniform(UNIFORM_MATERIAL_COLOR, color);
	shader->setUniform(UNIFORM_MATERIAL_SMOOTHNESS, spec_smooth);
	shader->setUniform(UNIFORM_MATERIAL_SPECULAR, spec_amount);

	shader->setUniform(UNIFORM_MATERIAL_DIFFUSE_TEXTURE, 0);
	if (diffuse_texture.isReady())
		diffuse_texture.Use2D(0);
	else
		Texture::UseWhite(0);

	shader->setUniform(UNIFORM_MATERIAL_NORMAL_TEXTURE, 1);
	if (normal_texture.isReady())
		normal_texture.Use2D(1);
	else
		Texture::UseBlankNormal(1);

	shader->setUniform(UNIFORM_MATERIAL_SPECULAR_TEXTURE, 2);
	if (specular_texture.isReady())
		specular_texture.Use2D(2);
	else
		Texture::UseWhite(2);

	shader->setUniform(UNIFORM_MATERIAL_GLOW_TEXTURE, 3);
	if (glow_texture.isReady())
		glow_texture.Use2D(3);
	else
		Texture::UseBlack(3);

	shader->setUniform(UNIFORM_SUNLIGHT_SHADOWMAP, 8);
//	shader->setUniform(UNIFORM_RANDOM_TEXTURE, 9);
//	Texture::UseRandom(9);

	return true;
}