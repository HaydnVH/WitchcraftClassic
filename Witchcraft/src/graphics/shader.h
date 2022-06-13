#ifndef HVH_WC_GRAPHICS_SHADER_H
#define HVH_WC_GRAPHICS_SHADER_H

#include "math/vmath.h"
#include <vector>
#include <string>

class Shader
{
public:
	Shader();
	~Shader();

	void FindUniformLocations(const std::vector<const char*>& uniform_names);
	bool areUniformsFound() { return (uniform_locations.size() != 0); }

	void setUniform(uint32_t location, float value);
	void setUniform(uint32_t location, float v0, float v1);
	void setUniform(uint32_t location, float v0, float v1, float v2);
	void setUniform(uint32_t location, float v0, float v1, float v2, float v3);
	void setUniform(uint32_t location, int value);
	void setUniform(uint32_t location, int v0, int v1);
	void setUniform(uint32_t location, int v0, int v1, int v2);
	void setUniform(uint32_t location, int v0, int v1, int v2, int v3);
	void setUniform(uint32_t location, vmath::vec2 value);
	void setUniform(uint32_t location, vmath::vec3 value);
	void setUniform(uint32_t location, vmath::vec4 value);
	void setUniform(uint32_t location, vmath::quat value);
	void setUniform(uint32_t location, vmath::vec2 row0, vmath::vec2 row1);
//	void setUniform(uint32_t location, const vmath::mat3& value);
	void setUniform(uint32_t location, const vmath::mat4& value);

	bool use();

	bool LoadSource(const char* vertsrc, const char* fragsrc, const char* geomsrc, const char* tescsrc, const char* tesesrc,
		std::vector<std::string>* vertdebug,
		std::vector<std::string>* fragdebug,
		std::vector<std::string>* geomdebug,
		std::vector<std::string>* tescdebug,
		std::vector<std::string>* tesedebug);

	uint32_t getProgram()
		{ return program; }

	static void InitDefaultShaders();
	static void Cleanup();

	static uint32_t RegisterUniformBuffer(const char* blockname);
	static void UploadUniformBuffer(uint32_t blockid, size_t size, void* ptr);

	static Shader* getShader(const char* shadername);

	static Shader* LoadGLSL(const char* filename, std::vector<const char*> defines = {});

private:

#ifdef RENDERER_OPENGL
	uint32_t program;
#endif

	bool ready;

	std::vector<uint32_t> uniform_locations;
};

#endif // HVH_WC_GRAPHICS_SHADER_H