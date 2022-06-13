#include "skybox.h"
#include "sys/printlog.h"

#include "filesystem/file_manager.h"

#include <string>
#include "tools/stringhelper.h"
#include "tools/fixedstring.h"
using namespace std;

#include "tools/xmlhelper.h"
using namespace pugi;

#include "tools/colors.h"
using namespace vmath;

#include "renderer.h"
#include "shader.h"
#include "light.h"

void Skybox::Load(const char* name)
{
	// Try to load a binary skybox file.
	char full_filename[64];
	snprintf(full_filename, 64, "%s%s", name, SKYBOX_BIN_FILE_EXT);
	InFile file = filemanager::LoadSingleFile(full_filename);
	if (!file.is_open())
	{
		// If that fails, load a text skybox file.
		snprintf(full_filename, 64, "%s%s", name, SKYBOX_TXT_FILE_EXT);
		file = filemanager::LoadSingleFile(full_filename);
		if (!file.is_open())
		{
			plog::error("Failed to load skybox '%s'.\n", name);
			return;
		}
	}

	string contents = file.contents();

	xml_document doc;
	xml_node root;

	xml_parse_result result = doc.load_buffer_inplace(&contents[0], contents.size());
	if (result)
		root = doc.first_child();

	if (root == NULL)
	{
		plog::error("Could not parse '%s'\n", full_filename);
		return;
	}

	xml_node node;
	xml_node subnode;

	// Get the cubemap textures
	vector<const char*> filenames;
	filenames.reserve(6);
	if (node = root.child("textures"))
	{
		filenames.push_back(readXML(node, "posx", "INVALID"));
		filenames.push_back(readXML(node, "negx", "INVALID"));
		filenames.push_back(readXML(node, "posy", "INVALID"));
		filenames.push_back(readXML(node, "negy", "INVALID"));
		filenames.push_back(readXML(node, "posz", "INVALID"));
		filenames.push_back(readXML(node, "negz", "INVALID"));
	}
	tex.LoadCubemapImages(&filenames[0], true, false, false);

	// Get the light properties
	if (node = root.child("light"))
	{
		if (subnode = node.child("color"))
		{
			colors::parse(subnode.text().as_string(), light_col);
		}

		if (subnode = node.child("ambient"))
		{
			colors::parse(subnode.text().as_string(), ambient_col);
			light_col -= ambient_col;
		}

		if (subnode = node.child("direction"))
		{
			sscanf(subnode.text().as_string(), "%f %f %f", &light_dir.x, &light_dir.y, &light_dir.z);
			light_dir.normalize();
		}
	}
	/*
	// Get the shadow properties from config.xml
	xml_node section = config::get_section("renderer");
	if ((node = section.child("shadows")) && (node = node.child("directional")) && node.child("is_enabled"))
	{
		uint32_t resolution = readXML(node, "resolution", 1024);

		if (node = node.child("cascades"))
		{
			istringstream ss(node.text().as_string());
			cascades = {
				istream_iterator<float>{ss},
				istream_iterator<float>{}
			};
		}

		shadow.Init(resolution, (uint32_t)cascades.size() + 1);
	}
	*/
	shadow.Init(2048, 4);
	cascades = {10.0f, 30.0f, 120.0f};


	VertexPosition vertices[] =
	{
		{-1, -1,  1},
		{ 1, -1,  1},
		{ 1,  1,  1},
		{-1,  1,  1},
		{-1, -1, -1},
		{ 1, -1, -1},
		{ 1,  1, -1},
		{-1,  1, -1}
	};

	uint8_t indices[] =
	{
		0, 2, 1, 2, 0, 3,
		7, 5, 6, 5, 7, 4,
		2, 3, 6, 6, 3, 7,
		0, 1, 4, 4, 1, 5,
		0, 4, 3, 4, 7, 3,
		1, 2, 5, 5, 2, 6,
	};
	
	geom.Load(GEOMETRY_PRIMITIVE_TRIANGLES, vertices, VF_POSITION, 8, indices, sizeof(uint8_t), 36);

	if (shader == NULL)
	{
		shader = Shader::getShader("skybox");
		shader->FindUniformLocations(SKYBOX_SHADER_UNIFORM_NAMES);
		light_buffer_id = Shader::RegisterUniformBuffer("LIGHTING");
	}
	
}

void Skybox::Draw(const UniformBufferCamera& cam)
{
	// Draw the skybox
	if (!shader || !shader->use())
		return;

	mat4 matrix = mat4::translation(cam.eye_pos.xyz);
	mat4 mvp = cam.projview * matrix;
	shader->setUniform(UNIFORM_MATRIX_MVP, mvp);
	mat4 transform = mat4::rotation(quat::euler(TO_RADIANS*90.0f, 0, 0));
	shader->setUniform(UNIFORM_MATRIX_TRANSFORM, transform);

	shader->setUniform(UNIFORM_TEXTURE_SKYBOX, 0);
	tex.UseCube(0);

	geom.Draw();
}

void Skybox::Lighting(const UniformBufferCamera& cam, float fov)
{
	// Handle skybox lighting
	UniformBufferLighting lighting_buffer = {};

	lighting_buffer.sunlight.l.direction = { light_dir, 0 };
	lighting_buffer.sunlight.l.diffuse_color = { light_col, 1.5f };
	lighting_buffer.sunlight.l.ambient_color = { ambient_col, 0 };

	lighting_buffer.num_dir_lights_used = 0;
	lighting_buffer.dir_lights[0].direction = { -light_dir, 0 };
	lighting_buffer.dir_lights[0].diffuse_color = { ambient_col, 0 };
	lighting_buffer.dir_lights[0].ambient_color = { 0, 0, 0, 0 };

	// Render the sky's shadow map
	vector<mat4> matrices = shadow.RenderDirectional(light_dir, cam, fov, cam.aspect_ratio, cascades);
	for (int i = 0; i < (int)matrices.size() && i <= 5; ++i)
	{
		lighting_buffer.sunlight.cascade_matrices[i] = matrices[i];
	}
	shadow.BindForRead(8);

	lighting_buffer.sunlight.num_cascades = (int)cascades.size() + 1;

	for (int i = 0; i < (int)cascades.size(); ++i)
	{
		lighting_buffer.sunlight.cascade_boundries[i].x = -cascades[i];
	}

//	component::light::SetPointLights(lighting_buffer);

	Shader::UploadUniformBuffer(light_buffer_id, sizeof(UniformBufferLighting), &lighting_buffer);
}