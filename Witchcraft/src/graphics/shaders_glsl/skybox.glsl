R"==(
#include inc/uniform_buffer_camera.glsl

#begin vertex shader

layout (location = 0) in vec3 in_position;

uniform mat4 mvp;
uniform mat4 transform;

out vec3 vert_texcoord;

void main()
{
	vec4 pos = mvp * vec4(in_position, 1);
	gl_Position = pos.xyww;
	vert_texcoord = (transform * vec4(in_position, 0)).xyz;
	vert_texcoord.y = -vert_texcoord.y;
}

#begin fragment shader

in vec3 vert_texcoord;

out vec4 frag_color;

uniform samplerCube skybox_texture;

void main()
{
	frag_color = texture(skybox_texture, normalize(vert_texcoord));
}
)=="
