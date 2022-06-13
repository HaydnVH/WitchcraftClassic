R"==(
#begin vertex shader

#include inc/uniform_buffer_camera.glsl

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 4) in vec2 in_texcoord;

out vec4 vert_color;
out vec2 vert_texcoord;

void main()
{
	// Map to normalized clip coordinates
	float x = ((2.0 * (in_position.x - 0.5)) / screen_size.x) - 1.0;
	float y = 1.0 - ((2.0 * (in_position.y - 0.5)) / screen_size.y);

	gl_Position = vec4(x, y, 0.0, 1.0);
	vert_texcoord = in_texcoord;
	vert_color = vec4(in_color.rgb, 1.0);
}

#begin fragment shader

in vec4 vert_color;
in vec2 vert_texcoord;

out vec4 frag_color;

uniform sampler2D glyph_texture;

void main()
{
	frag_color = vert_color;
	frag_color.a = texture(glyph_texture, vert_texcoord).r;
}

)=="