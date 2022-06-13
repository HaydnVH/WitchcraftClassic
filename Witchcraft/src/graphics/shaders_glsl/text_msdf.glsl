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
//out vec2 vert_cutoffs;

void main()
{
	gl_Position = vec4((in_position_texcoord.xy+position) * vec2(2, -2), 0, 1);
	gl_Position.xy += vec2(-1, 1);
	vert_texcoord = in_position_texcoord.zw / textureSize(tex_glyphs, 0);

	// The index of this glyph, relative to the beginning of the string (?)
	float charnum = float(gl_VertexID / 6);

	// Here we handle gradient colors.
	// To disable gradients, just make in_color1 and in_color2 the same.
	// If span.x == span.y, a divide-by-zero error will occur.
//	float t = linstep(in_position_textcoord.y, in_gradient_span.x, in_gradient_span.y);
//	vert_color = mix(in_color1, in_color2, t);

//	vert_cutoffs = in_cutoffs;
	vert_color = vec4(1, 1, 1, 1);

}

#begin fragment shader

in vec4 vert_color;
in vec2 vert_texcoord;
//in vec2 vert_cutoffs;

out vec4 frag_color;

uniform vec4 color;
uniform vec2 cutoffs;
uniform vec4 cliprect;

void main()
{
	frag_color = color;
	float distval = median(texture(tex_glyphs, vert_texcoord).rgb);
	
	/*
	vec2 one_texel = vec2(1,1) / vec2(textureSize(tex_glyphs, 0));
	float up = linstep(median(texture(tex_glyphs, vert_texcoord + vec2(0,one_texel.y)).rgb), cutoffs.x, 1);
	float down = linstep(median(texture(tex_glyphs, vert_texcoord - vec2(0,one_texel.y)).rgb), cutoffs.x, 1);
	float right = linstep(median(texture(tex_glyphs, vert_texcoord + vec2(one_texel.x, 0)).rgb), cutoffs.x, 1);
	float left = linstep(median(texture(tex_glyphs, vert_texcoord - vec2(one_texel.x, 0)).rgb), cutoffs.x, 1);
	vec3 normal = vec3(right-left, down-up, 0);
	normal.z = sqrt(1-dot(normal.xy, normal.xy));

	// Do lighting!
	vec3 light_dir = normalize(vec3(0.75, -1, 1));
	float NdotL = dot(normal, light_dir);
	vec3 light = vec3(0.1, 0.1, 0.1) + (vec3(0.9, 0.9, 0.9) * max(0, NdotL));
	frag_color.rgb *= light;
	*/
	
	frag_color.a *= linstep(distval, cutoffs.x, cutoffs.y);

	vec2 pos = gl_FragCoord.xy / screen_size;
	vec4 clip = vec4(cliprect.x, 1-cliprect.y, cliprect.z, 1-cliprect.w);
	frag_color.a = ((pos.x < clip.x) || (pos.y > clip.y) || (pos.x > clip.z) || (pos.y < clip.w)) ? 0 : frag_color.a;
}

)=="