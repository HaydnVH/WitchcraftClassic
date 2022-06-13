R"==(
#begin vertex shader

#include inc/uniform_buffer_camera.glsl

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;

out vec4 vert_color;
out vec4 viewspace;

void main()
{
	gl_Position = projview * vec4(in_position, 1.0);
	viewspace = view * vec4(in_position, 1.0);
	vert_color = in_color;
}

#begin fragment shader

in vec4 vert_color;
in vec4 viewspace;
out vec4 frag_color;

uniform float transparency;

float linstep(float min, float max, float v)
{
	return clamp((v-min) / (max-min), 0, 1);
}

void main()
{
	frag_color = vec4(vert_color.rgb, 1.0);
	frag_color.a *= transparency;

	float dist = length(viewspace);
	float fogfactor = 1 - linstep(1, 25, dist);

	frag_color.a *= fogfactor;
}
)=="