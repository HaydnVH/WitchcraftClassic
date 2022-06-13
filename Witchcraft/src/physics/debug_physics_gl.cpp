#ifdef RENDERER_OPENGL
#include "debug_physics.h"

#include <GL/glew.h>
#include <GL/GL.h>

#include "graphics/shader.h"

#include "sys/printlog.h"

constexpr const int DEFAULT_NUM_LINES = 4096;

void PhysicsDebug::Init()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DebugLineSegment) * DEFAULT_NUM_LINES, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugLineSegment::from), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(DebugLineSegment::to), (void*)12);

	glBindVertexArray(0);
}

PhysicsDebug::~PhysicsDebug()
{
	if (vbo)
		glDeleteBuffers(1, &vbo);

	if (vao)
		glDeleteBuffers(1, &vao);
}

void PhysicsDebug::Draw(Shader* shader, float interpolation)
{
	if (is_enabled == false)
		return;

	if (shader == nullptr || shader->use() == false)
		return;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	static size_t num_lines = DEFAULT_NUM_LINES;

	if (lines.size() > num_lines)
	{
		num_lines = ((lines.size() / DEFAULT_NUM_LINES) + 1) * DEFAULT_NUM_LINES;
		// expand the size of the vertex buffer here
		glBufferData(GL_ARRAY_BUFFER, sizeof(DebugLineSegment) * num_lines, nullptr, GL_DYNAMIC_DRAW);

		plog::info("Expanded size of debug line vertex buffer to hold %i lines.\n", num_lines);
	}

	// If the size of the lines array hasn't changed since the last frame, we assume that the objects being drawn are the same.
	// Therefore, we can interpolate the line positions.
	if (old_lines.size() == lines.size())
	{
		std::vector<DebugLineSegment> new_lines(lines.size());
		for (size_t i = 0; i < lines.size(); ++i)
		{
			new_lines[i] = lines[i];
			new_lines[i].from.pos = vmath::vec3::lerp(old_lines[i].from.pos, lines[i].from.pos, interpolation);
			new_lines[i].to.pos = vmath::vec3::lerp(old_lines[i].to.pos, lines[i].to.pos, interpolation);
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(DebugLineSegment) * new_lines.size(), new_lines.data() );
	}
	else
	{
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(DebugLineSegment) * lines.size(), lines.data() );
	}


	// Draw lines which are not obscured by the depth buffer.
	glDepthFunc(GL_LEQUAL);
	shader->setUniform(0, 1.0f);
	glDrawArrays(GL_LINES, 0, (GLsizei)(lines.size() * 2));

	// Draw lines which are obscured by the depth buffer.
	glDepthFunc(GL_GREATER);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	shader->setUniform(0, 0.125f);
	glDrawArrays(GL_LINES, 0, (GLsizei)(lines.size() * 2));
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	glDepthFunc(GL_LESS);
	glBindVertexArray(0);
}

void PhysicsDebug::reportErrorWarning(const char* str)
{
	plog::warning("Bullet warning generated:\n%s", str);
}

#endif //RENDERER_OPENGL