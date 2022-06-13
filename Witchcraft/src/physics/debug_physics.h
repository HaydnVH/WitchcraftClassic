#ifndef HVH_WC_PHYSICS_DEBUGPHYSICS_H
#define HVH_WC_PHYSICS_DEBUGPHYSICS_H

#include "math/vmath.h"
#include <btBulletDynamicsCommon.h>

#include <vector>

class Shader;

struct DebugLineSegment
{
	struct { vmath::vec3 pos; uint8_t r, g, b, a; } from;
	struct { vmath::vec3 pos; uint8_t r, g, b, a; } to;
};

class PhysicsDebug : public btIDebugDraw
{
public:
	PhysicsDebug() {};
	~PhysicsDebug();

	void Init();

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		if (is_enabled == false)
			return;

		lines.push_back({	{{from.x(), from.y(), from.z()},
							(uint8_t)(color.x()*255.0f), (uint8_t)(color.y()*255.0f), (uint8_t)(color.z()*255.0f), 255},
							{{to.x(), to.y(), to.z()},
							(uint8_t)(color.x()*255.0f), (uint8_t)(color.y()*255.0f), (uint8_t)(color.z()*255.0f), 255} });
	}
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& from_color, const btVector3& to_color)
	{
		if (is_enabled == false)
			return;

		lines.push_back({	{{from.x(), from.y(), from.z()},
							(uint8_t)(from_color.x()*255.0f), (uint8_t)(from_color.y()*255.0f), (uint8_t)(from_color.z()*255.0f), 255},
							{{to.x(), to.y(), to.z()},
							(uint8_t)(to_color.x()*255.0f), (uint8_t)(to_color.y()*255.0f), (uint8_t)(to_color.z()*255.0f), 255} });
	}

	void drawContactPoint(const btVector3& point, const btVector3& normal, btScalar dist, int lifetime, const btVector3& color) {}
	void reportErrorWarning(const char* str);
	void draw3dText(const btVector3& location, const char* str) {}
	void setDebugMode(int mode) { debug_mode = mode; }
	int getDebugMode() const { return debug_mode; }

	void clear() { old_lines = lines; lines.clear(); }
	void Draw(Shader* shader, float interpolation);

	void setEnabled(bool enabled)
		{ is_enabled = enabled; }

private:

	bool is_enabled = true;

	std::vector<DebugLineSegment> lines;
	std::vector<DebugLineSegment> old_lines;
	uint32_t vbo = 0, vao = 0;

	int debug_mode = DBG_DrawWireframe;
};

#endif // HVH_WC_PHYSICS_DEBUGPHYSICS_H