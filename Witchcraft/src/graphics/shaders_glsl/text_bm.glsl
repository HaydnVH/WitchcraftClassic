R"==(

uniform sampler2D tex_glyphs;
//uniform sampler2D tex_random;

#include inc/extramath.glsl
#include inc/uniform_buffer_camera.glsl

#begin vertex shader


layout (location = 0) in vec4 in_position_texcoord;
//layout (location = 1) in vec4 in_color1;
//layout (location = 2) in vec4 in_color2;
//layout (location = 3) in vec2 in_gradient_span;
//layout (location = 4) in vec2 in_cutoffs;
//layout (location = 5) in vec4 in_waveshake;

uniform vec2 position;

out vec4 vert_color;
out vec2 vert_texcoord;

void main()
{
	gl_Position = vec4((in_position_texcoord.xy / screen_size), 0, 1);
	gl_Position.xy *= vec2(2, -2);
	gl_Position.xy += vec2(-1, 1);

	vert_texcoord = in_position_texcoord.zw / textureSize(tex_glyphs, 0);

	// The index of this glyph, relative to the beginning of the string (?)
	float charnum = float(gl_VertexID / 6);

	// Here we handle gradient colors.
	// To disable gradients, just make in_color1 and in_color2 the same.
	// If span.x == span.y, a divide-by-zero error will occur.
//	float t = linstep(in_position_textcoord.y, in_gradient_span.x, in_gradient_span.y);
//	vert_color = mix(in_color1, in_color2, t);

	vert_color = vec4(1, 1, 1, 1);

}

#begin fragment shader

in vec4 vert_color;
in vec2 vert_texcoord;

out vec4 frag_color;

uniform vec4 color;
uniform vec2 cutoffs;
uniform vec4 cliprect;

uniform float outline;

void main()
{
	frag_color = color;
	frag_color.a *= clamp((texture(tex_glyphs, vert_texcoord).r * 2) - (1 - outline), 0, 1);

	vec2 pos = gl_FragCoord.xy / screen_size;
	vec4 clip = vec4(cliprect.x, 1-cliprect.y, cliprect.z, 1-cliprect.w);
	frag_color.a = ((pos.x < clip.x) || (pos.y > clip.y) || (pos.x > clip.z) || (pos.y < clip.w)) ? 0 : frag_color.a;
}

)=="