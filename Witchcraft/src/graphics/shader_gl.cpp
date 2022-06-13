#ifdef RENDERER_OPENGL
#include "shader.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include "sys/printlog.h"
#include "sys/timer.h"
#include <unordered_map>
#include <set>
using namespace std;

namespace {

	unordered_map<string, string> available_src;
	unordered_map<string, Shader*> shaders;

	unordered_map<string, uint32_t> uniform_buffer_name_to_index;
	vector<uint32_t> uniform_buffer_objects;

	enum ShaderType
	{
		SHADERTYPE_INVALID = 0,
		SHADERTYPE_COMMON,
		SHADERTYPE_VERTEX,
		SHADERTYPE_FRAGMENT,
		SHADERTYPE_GEOMETRY,
		SHADERTYPE_TESSELATION_CONTROL,
		SHADERTYPE_TESSELATION_EVALUATION,
		SHADERTYPE_MAXENUM
	};

	void LoadGLSLRecursive(const char* filename, set<string>& opened_files, vector<ShaderType>& shadertype_stack,
		string& common_src, string& vert_src, string& frag_src, string& geom_src, string& tesc_src, string& tese_src,
		vector<string>& common_debug, vector<string>& vert_debug, vector<string>& frag_debug, vector<string>& geom_debug, vector<string>& tesc_debug, vector<string>& tese_debug)
	{
		// If this file has already been loaded before, cancel now to prevent an infinite loop.
		// This may not actually be an error.  If we have "standard includes", do we want multiple files #include'ing them?
		if (opened_files.count(filename) != 0)
			{ return; }

		if (available_src.count(filename) == 0)
		{
			plog::error("In Shader:LoadGLSLRecursive():\n");
			plog::errmore("Requested source '%s' does not exist.\n", filename);
			return;
		}

		// Make sure we can't load this file again later.
		opened_files.insert(filename);

		// Read the file.
		stringstream file(available_src[filename]);
		string current_line;
		unsigned int line_number = 0;

		while (getline(file, current_line))
		{
			++line_number;
			const char* curline = current_line.c_str();

			// Handle '#begin ___'
			if (strncmp(curline, "#begin ", 7) == 0)
			{
				if (strncmp(curline + 7, "vertex shader", 13) == 0)
					shadertype_stack.push_back(SHADERTYPE_VERTEX);

				else if (strncmp(curline + 7, "fragment shader", 15) == 0)
					shadertype_stack.push_back(SHADERTYPE_FRAGMENT);

				else if (strncmp(curline + 7, "geometry shader", 15) == 0)
					shadertype_stack.push_back(SHADERTYPE_GEOMETRY);

				else if (strncmp(curline + 7, "tesselation control shader", 26) == 0)
					shadertype_stack.push_back(SHADERTYPE_TESSELATION_CONTROL);

				else if (strncmp(curline + 7, "tesselation evaluation shader", 29) == 0)
					shadertype_stack.push_back(SHADERTYPE_TESSELATION_EVALUATION);

				else if (strncmp(curline + 7, "common", 6) == 0)
					shadertype_stack.push_back(SHADERTYPE_COMMON);

				continue;
			}

			// Handle '#end'
			else if ((strncmp(curline, "#end", 4) == 0) &&
				strncmp(curline, "#endif", 6) != 0)
			{
				if (shadertype_stack.size() > 0)
					shadertype_stack.pop_back();
				else
				{
					plog::error("In Shader::LoadGLSLRecursive(), while loading '%s':\n", filename);
					plog::errmore("Encountered '#end' without an associated '#begin'.\n");
				}
				continue;
			}

			// Handle '#include'
			else if (strncmp(curline, "#include ", 9) == 0)
			{
				char path[64];
				memset(path, 0, sizeof(path));
				sscanf(curline + 9, "%s", path);

				// This is a recursive call.  The "base case" is when you parse a file with no "#include"'s in it.
				// This function will automatically fail if it encounters a file that has already been parsed anywhere along the way.
				LoadGLSLRecursive(path, opened_files, shadertype_stack,
					common_src, vert_src, frag_src, geom_src, tesc_src, tese_src,
					common_debug, vert_debug, frag_debug, geom_debug, tesc_debug, tese_debug);

				continue;
			}

			else
			{
				current_line += '\n';
			}

			// Now that we have our line, lets add it (and the debug information) to the correct shader source.
			ShaderType current_src = SHADERTYPE_COMMON;
			if (shadertype_stack.size() > 0)
				current_src = shadertype_stack.back();

			char debugline[128];
			snprintf(debugline, 128, "%s(%u)", filename, line_number);

			switch (current_src)
			{
			case SHADERTYPE_VERTEX:
				vert_src += current_line;
				vert_debug.push_back(debugline);
				break;
			case SHADERTYPE_FRAGMENT:
				frag_src += current_line;
				frag_debug.push_back(debugline);
				break;
			case SHADERTYPE_GEOMETRY:
				frag_src += current_line;
				frag_debug.push_back(debugline);
				break;
			case SHADERTYPE_TESSELATION_CONTROL:
				tesc_src += current_line;
				tesc_debug.push_back(debugline);
				break;
			case SHADERTYPE_TESSELATION_EVALUATION:
				tese_src += current_line;
				tese_debug.push_back(debugline);
				break;
			case SHADERTYPE_COMMON:
				common_src += current_line;
				common_debug.push_back(debugline);
				break;
			}
		}
	}

	uint32_t LoadShaderFromSource(uint32_t prog, const char* src, GLenum type, const vector<string>* debug)
	{
		// Create the shader object
		uint32_t handle = glCreateShader(type);

		// Load the shader source code.
		glShaderSource(handle, 1, &src, nullptr);

		// Compile the shader.
		glCompileShader(handle);

		// Check for errors.
		int success = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			int logsize = 0;
			glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logsize);
			string infolog(logsize, '\0');
			glGetShaderInfoLog(handle, logsize, nullptr, &infolog[0]);

			// TODO: Find a better way to handle error reports.
			// OpenGL info log errors are formatted as "0(#) : ..." where # is the line number.
			// We need to search for any instances of that pattern and replace it.
			if (debug)
			{
				size_t pos1 = 0;
				if (infolog.find("ERROR: ") != string::npos)
				{
					while (true)
					{
						// Search for "ERROR: " which proceeds every error.
						pos1 = infolog.find("ERROR: ", pos1);
						if (pos1 == string::npos)
							break;

						pos1 += 7;
						size_t pos2 = infolog.find(": ", pos1);
						string sub = infolog.substr(pos1, pos2 - pos1);

						unsigned int line = 0;

						// If it matches the pattern "#:#", then it's an error we can translate.
						if (sscanf(sub.c_str(), "%*u:%u", &line) == 1)
						{
							// Replace the meaninless line info with something relevant.
							if (line - 1 <= debug->size())
								infolog.replace(pos1, pos2 - pos1, (*debug)[line - 1]);
						}

						pos1 = pos2;
					}
				}
				else if (infolog.find(" : error ") != string::npos)
				{
					while (true)
					{
						pos1 = infolog.find(" : error ", pos1);
						if (pos1 == string::npos)
							break;

						// Back up out position until the char behind it is out of bounds or a newline.
						size_t pos2 = pos1;
						while (pos2 > 0 && infolog[pos2 - 1] != '\n')
						{
							--pos2;
						}

						string sub = infolog.substr(pos2, pos1 - pos2);

						unsigned int line = 0;

						if (sscanf(sub.c_str(), "%*u(%u)", &line) == 1)
						{
							// Replace the meaningless line info with something useful
							if (line - 1 <= debug->size())
								infolog.replace(pos2, pos1 - pos2, (*debug)[line - 1]);
						}

						pos1 += 5;
					}
				}
				else
				{
					// Unknown error formatting.
				}
			}

			plog::error("In ShaderManager::loadShaderFromSource():\n");
			plog::errmore("Error compiling shader:\n%s\n", infolog.c_str());

			glDeleteShader(handle);
			return 0;
		}

		// Attach the shader.
		glAttachShader(prog, handle);
		return handle;
	}

	bool LinkShaders(uint32_t prog)
	{
		// Link the program.
		glLinkProgram(prog);

		// Check for errors.
		int success = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &success);
		if (!success)
		{
			int logsize = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logsize);
			string infolog(logsize, '\0');
			glGetProgramInfoLog(prog, logsize, nullptr, &infolog[0]);

			plog::error("In ShaderManager::LinkShader():\n");
			plog::errmore("Error linking shader:\n%s\n", infolog.c_str());

			return false;
		}

		return true;
	}

} // namespace <anon>

Shader::Shader()
:	program(0),
	ready(false),
	uniform_locations()
{}

Shader::~Shader()
{
	if (program)
		glDeleteProgram(program);
}

bool Shader::LoadSource(const char* vertsrc, const char* fragsrc, const char* geomsrc, const char* tescsrc, const char* tesesrc,
	vector<string>* vertdebug, vector<string>* fragdebug, vector<string>* geomdebug, vector<string>* tescdebug, vector<string>* tesedebug)
{
	if ((!vertsrc) || (!fragsrc))
	{
		plog::error("In Shader::LoadSource():\n");
		plog::errmore("Vertex shader and fragment shader must be provided.\n");
		return false;
	}


	if (program)
		glDeleteProgram(program);

	program = glCreateProgram();

	uint32_t vert = 0, frag = 0, geom = 0, tesc = 0, tese = 0;

	vert = LoadShaderFromSource(program, vertsrc, GL_VERTEX_SHADER, vertdebug);
	frag = LoadShaderFromSource(program, fragsrc, GL_FRAGMENT_SHADER, fragdebug);

	if (geomsrc)
		geom = LoadShaderFromSource(program, geomsrc, GL_GEOMETRY_SHADER, geomdebug);
	if (tescsrc)
		tesc = LoadShaderFromSource(program, tescsrc, GL_TESS_CONTROL_SHADER, tescdebug);
	if (tesesrc)
		tese = LoadShaderFromSource(program, tesesrc, GL_TESS_EVALUATION_SHADER, tesedebug);

	bool success = LinkShaders(program);

	if (vert)
		{ glDetachShader(program, vert); glDeleteShader(vert); }

	if (frag)
		{ glDetachShader(program, frag); glDeleteShader(frag); }

	if (geom)
		{ glDetachShader(program, geom); glDeleteShader(geom); }
	
	if (tesc)
		{ glDetachShader(program, tesc); glDeleteShader(tesc); }
	
	if (tese)
		{ glDetachShader(program, tese); glDeleteShader(tese); }

	ready = success;
	return success;
}

bool Shader::use()
{
	static uint32_t active_program = 0;

	if (!program || !ready)
		return false;

	if (program == active_program)
		return true;
	
	glUseProgram(program);
	return true;
}

void Shader::FindUniformLocations(const vector<const char*>& uniform_names)
{
	if (uniform_locations.size() != 0)
	{
		plog::warning("Finding uniform locations for shader which already has uniform locations.\n");
	}

	uniform_locations.resize(uniform_names.size());

	for (size_t i = 0; i < uniform_names.size(); ++i)
	{
		uniform_locations[i] = glGetUniformLocation(program, uniform_names[i]);
	}
}

void Shader::setUniform(uint32_t location, float value)
	{ glUniform1f(uniform_locations[location], value); }

void Shader::setUniform(uint32_t location, float v0, float v1)
	{ glUniform2f(uniform_locations[location], v0, v1); }

void Shader::setUniform(uint32_t location, float v0, float v1, float v2)
	{ glUniform3f(uniform_locations[location], v0, v1, v2); }

void Shader::setUniform(uint32_t location, float v0, float v1, float v2, float v3)
	{ glUniform4f(uniform_locations[location], v0, v1, v2, v3); }

void Shader::setUniform(uint32_t location, int value)
	{ glUniform1i(uniform_locations[location], value); }

void Shader::setUniform(uint32_t location, int v0, int v1)
	{ glUniform2i(uniform_locations[location], v0, v1); }

void Shader::setUniform(uint32_t location, int v0, int v1, int v2)
	{ glUniform3i(uniform_locations[location], v0, v1, v2); }

void Shader::setUniform(uint32_t location, int v0, int v1, int v2, int v3)
	{ glUniform4i(uniform_locations[location], v0, v1, v2, v3); }

void Shader::setUniform(uint32_t location, vmath::vec2 value)
	{ glUniform2fv(uniform_locations[location], 1, value.data); }

void Shader::setUniform(uint32_t location, vmath::vec3 value)
	{ glUniform3fv(uniform_locations[location], 1, value.data); }

void Shader::setUniform(uint32_t location, vmath::vec4 value)
	{ glUniform4fv(uniform_locations[location], 1, value.data); }

void Shader::setUniform(uint32_t location, vmath::quat value)
	{ glUniform4fv(uniform_locations[location], 1, value.data); }

void Shader::setUniform(uint32_t location, vmath::vec2 row0, vmath::vec2 row1)
{
	vmath::vec4 mat2 = { row0.x, row0.y, row1.x, row1.y };
	glUniformMatrix2fv(uniform_locations[location], 1, false, mat2.data);
}

//void Shader::setUniform(uint32_t location, const vmath::mat3& value)
//	{ glUniformMatrix3fv(uniform_locations[location], 1, false, value.data); }

void Shader::setUniform(uint32_t location, const vmath::mat4& value)
	{ glUniformMatrix4fv(uniform_locations[location], 1, false, value.data); }


uint32_t Shader::RegisterUniformBuffer(const char* block_name)
{
	// If we're requesting a uniform buffer that already exists, we simply return it.
	if (uniform_buffer_name_to_index.count(block_name) != 0)
	{
		plog::warning("Registering uniform buffer '%s' which has already been registered.\n", block_name);
		return uniform_buffer_objects[uniform_buffer_name_to_index[block_name]];
	}

	// Create the new uniform buffer.
	uint32_t new_buffer = 0;
	glGenBuffers(1, &new_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	uint32_t buffer_index = (uint32_t)uniform_buffer_objects.size();

	for (auto& it : shaders)
	{
		Shader* ptr = it.second;
		if (!ptr) continue;

		uint32_t block_index = glGetUniformBlockIndex(ptr->getProgram(), block_name);
		if (block_index != GL_INVALID_INDEX)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, buffer_index, new_buffer);
			glUniformBlockBinding(ptr->getProgram(), block_index, buffer_index);
		}
	}

	// Remember the buffer index and associate it with the name of the buffer.
	uniform_buffer_name_to_index[block_name] = buffer_index;
	uniform_buffer_objects.push_back(new_buffer);

	return new_buffer;
}

void Shader::UploadUniformBuffer(uint32_t buffer_id, size_t size, void* ptr)
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
	glBufferData(GL_UNIFORM_BUFFER, size, ptr, GL_DYNAMIC_DRAW);
}


//// Static stuff ////


Shader* Shader::LoadGLSL(const char* filename, vector<const char*> defines)
{
	// The shader to return to the user
	Shader* result = NULL;

	// Construct the string that'll be used to identify this particular shader
	string idstring = filename;
	for (const string& str : defines)
	{
		idstring += "|" + str;
	}

	// Store the source code for each shader type
	string common_src, vert_src, frag_src, geom_src, tesc_src, tese_src;

	// Debugging information
	vector<string> common_debug, vert_debug, frag_debug, geom_debug, tesc_debug, tese_debug;

	// Handle defines
	common_src += "#version 330\n";
	common_debug.push_back("Engine-Defined(#version 330)");
	for (const string& str : defines)
	{
		common_src += "#define " + str + '\n';
		common_debug.push_back("Engine-Defined(#define " + str + ")");
	}

	// Load the file.
	set<string> opened_files;
	vector<ShaderType> shadertype_stack;
	LoadGLSLRecursive(filename, opened_files, shadertype_stack,
		common_src, vert_src, frag_src, geom_src, tesc_src, tese_src,
		common_debug, vert_debug, frag_debug, geom_debug, tesc_debug, tese_debug);

	// Make sure we actually have vertex and fragment source code
	if ((vert_src.size() == 0 || frag_src.size() == 0))
	{
		plog::error("Failed to load shader '%s'.\n", filename);
		plog::errmore("All shaders must have a vertex shader and fragment shader!\n");
		return NULL;
	}

	// Concatenate debug infos
	vector<string> cVertDebug = common_debug; cVertDebug.insert(cVertDebug.end(), vert_debug.begin(), vert_debug.end());
	vector<string> cFragDebug = common_debug; cFragDebug.insert(cFragDebug.end(), frag_debug.begin(), frag_debug.end());
	vector<string> cGeomDebug = common_debug; cGeomDebug.insert(cGeomDebug.end(), geom_debug.begin(), geom_debug.end());
	vector<string> cTescDebug = common_debug; cTescDebug.insert(cTescDebug.end(), tesc_debug.begin(), tesc_debug.end());
	vector<string> cTeseDebug = common_debug; cTeseDebug.insert(cTeseDebug.end(), tese_debug.begin(), tese_debug.end());

	result = new Shader;

	// Actually create the new shader using our source code
	if (result->LoadSource(
		vert_src.size() > 0 ? (common_src + vert_src).c_str() : NULL,
		frag_src.size() > 0 ? (common_src + frag_src).c_str() : NULL,
		geom_src.size() > 0 ? (common_src + geom_src).c_str() : NULL,
		tesc_src.size() > 0 ? (common_src + tesc_src).c_str() : NULL,
		tese_src.size() > 0 ? (common_src + tese_src).c_str() : NULL,
		&cVertDebug, &cFragDebug, &cGeomDebug, &cTescDebug, &cTeseDebug) == false)
	{
		plog::error("In Shader::LoadShader():\n");
		plog::errmore("Failed to load shader '%s'.\n", idstring.c_str());
		delete result;
		return NULL;
	}

	return result;
}

void Shader::InitDefaultShaders()
{
	plog::info("Compiling shaders...\n");
	sys::Timer timer; timer.update();

	// Load in the glsl source code for default shaders.
	available_src["inc/uniform_buffer_camera.glsl"] =
#include "shaders_glsl/inc/uniform_buffer_camera.glsl"
	; available_src["inc/lighting.glsl"] =
#include "shaders_glsl/inc/lighting.glsl"
	; available_src["inc/blinn_phong.brdf"] =
#include "shaders_glsl/inc/blinn_phong.brdf"
	; available_src["inc/skeletal_animation.glsl"] =
#include "shaders_glsl/inc/skeletal_animation.glsl"
	; available_src["inc/quaternion.glsl"] =
#include "shaders_glsl/inc/quaternion.glsl"
	; available_src["inc/extramath.glsl"] =
#include "shaders_glsl/inc/extramath.glsl"
	;

	available_src["debugdraw.glsl"] =
#include "shaders_glsl/debugdraw.glsl"
	; available_src["debugtext.glsl"] =
#include "shaders_glsl/debugtext.glsl"
	; available_src["standard_material.glsl"] =
#include "shaders_glsl/standard_material.glsl"
	; available_src["skybox.glsl"] =
#include "shaders_glsl/skybox.glsl"
	; available_src["shadow.glsl"] =
#include "shaders_glsl/shadow.glsl"
	; available_src["text_msdf.glsl"] =
#include "shaders_glsl/text_msdf.glsl"
	; available_src["text_bm.glsl"] =
#include "shaders_glsl/text_bm.glsl"
	;

	// Compile the default shaders.
	Shader* shader = nullptr;

	// debugdraw
	if ((shader = LoadGLSL("debugdraw.glsl", {})) != nullptr)
		{ shaders["debugdraw"] = shader; }

	// debugtext
	if ((shader = LoadGLSL("debugtext.glsl", {})) != nullptr)
		{ shaders["debugtext"] = shader; }

	// standard material
	if ((shader = LoadGLSL("standard_material.glsl", {"TEX_DIFFUSE", "TEX_NORMAL", "TEX_SPECULAR", "TEX_GLOW"})) != nullptr)
		{ shaders["standard_material"] = shader; }

	// standard material, skinned
	if ((shader = LoadGLSL("standard_material.glsl", {"TEX_DIFFUSE", "TEX_NORMAL", "TEX_SPECULAR", "TEX_GLOW", "SKELETAL_ANIMATION"})) != nullptr)
		{ shaders["standard_material|SKELETAL_ANIMATION"] = shader; }

	// Skybox
	if (shader = LoadGLSL("skybox.glsl", {}))
		{ shaders["skybox"] = shader; }

	// Shadows
	if (shader = LoadGLSL("shadow.glsl", {}))
		{ shaders["shadow"] = shader; }
	if (shader = LoadGLSL("shadow.glsl", {"SKELETAL_ANIMATION"}))
		{ shaders["shadow|SKELETAL_ANIMATION"] = shader; }

	// Text
	if (shader = LoadGLSL("text_msdf.glsl", {}))
		{ shaders["text_msdf"] = shader; }
	if (shader = LoadGLSL("text_bm.glsl", {}))
		{ shaders["text_bm"] = shader; }

	timer.update();
	plog::info("Finished compiling shaders (took %f ms).\n", timer.getDeltaTime() * 1000.0);
}

void Shader::Cleanup()
{
	for (auto& it : shaders)
	{
		delete it.second;
		it.second = nullptr;
	}
}

Shader* Shader::getShader(const char* shadername)
{
	if (shaders.count(shadername) == 0)
		return nullptr;
	else
		return shaders[shadername];
}

#endif // RENDERER_OPENGL