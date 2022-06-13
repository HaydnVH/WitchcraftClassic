R"==(
#include inc/lighting.glsl
#include inc/blinn_phong.brdf

#include inc/uniform_buffer_camera.glsl

#begin vertex shader

uniform mat4 mvp;
uniform mat4 transform;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texcoord;
layout (location = 2) in vec4 in_normal;
layout (location = 3) in vec4 in_tangent;

#include inc/skeletal_animation.glsl

out vec3 vert_normal;
#ifdef TEX_NORMAL
out vec3 vert_tangent;
out vec3 vert_bitangent;
#endif
out vec2 vert_texcoord;
out float vert_color;

void main()
{
#ifdef SKELETAL_ANIMATION
	
	vec3 skinned_pos = ApplyBoneTransform(vec4(in_position.xyz, 1)).xyz;
	gl_Position = mvp * vec4(skinned_pos, 1);
	vec3 skinned_nrm = ApplyBoneTransform(vec4(in_normal.xyz, 0)).xyz;
	vert_normal = (transform * vec4(skinned_nrm, 0)).xyz;

	#ifdef TEX_NORMAL
		vec3 skinned_tan = ApplyBoneTransform(vec4(in_tangent.xyz, 0)).xyz;
		vert_tangent = (transform * vec4(skinned_tan, 0)).xyz;
		vert_bitangent = cross(vert_normal, vert_tangent) * in_tangent.w;
	#endif // TEX_NORMAL
		
	CalcVertexLightVals(transform, skinned_pos);
	
#else // not SKELETAL_ANIMATION

	gl_Position = mvp * vec4(in_position, 1);
	vert_normal = (transform * vec4(in_normal.xyz, 0)).xyz;

	#ifdef TEX_NORMAL
		vert_tangent = (transform * vec4(in_tangent.xyz, 0)).xyz;
		vert_bitangent = cross(vert_normal, vert_tangent) * in_tangent.w;
	#endif // TEX_NORMAL
	
	#ifndef SKIP_LIGHTING
		CalcVertexLightVals(transform, in_position);
	#endif // SKIP_LIGHTING

#endif // SKELETAL_ANIMATION


	vert_texcoord = in_texcoord;
	vert_color = in_normal.w;
	
}

#begin fragment shader

struct Material
{
	vec4 color;
	float smoothness;
	float specular;

#ifdef TEX_DIFFUSE
	sampler2D diffuse_texture;
#endif
#ifdef TEX_NORMAL
	sampler2D normal_texture;
#endif
#ifdef TEX_SPECULAR
	sampler2D specular_texture;
#endif
#ifdef TEX_GLOW
	sampler2D glow_texture;
#endif
};
uniform Material material;

in vec3 vert_normal;
in vec2 vert_texcoord;
in float vert_color;

out vec4 frag_color;

#ifdef TEX_NORMAL
in vec3 vert_tangent;
in vec3 vert_bitangent;
vec3 CalcBumpedNormal()
{
	vec3 normal = normalize(vert_normal);
	vec3 tangent = normalize(vert_tangent);
	vec3 bitangent = normalize(vert_bitangent);
	
	vec3 bumpmapnormal = texture(material.normal_texture, vert_texcoord).xyz;
	bumpmapnormal = 2 * bumpmapnormal - 1;
	
	vec3 newnormal;
	mat3 tbn = mat3(tangent, bitangent, normal);
	newnormal = tbn * bumpmapnormal;
	return normalize(newnormal);
}
#endif // TEX_NORMAL

void main()
{	
	frag_color = material.color * vert_color;
	#ifdef TEX_DIFFUSE
		frag_color *= texture(material.diffuse_texture, vert_texcoord);
	#endif

	#ifdef TEX_NORMAL
		vec3 normal = CalcBumpedNormal();
	#else
		vec3 normal = normalize(vert_normal);
	#endif

	vec3 specular = vec3(material.specular, material.specular, material.specular);
	#ifdef TEX_SPECULAR
		specular *= texture(material.specular_texture, vert_texcoord).rgb;
	#endif

	vec3 light = CalcLighting(normal, specular, material.smoothness);
	frag_color.rgb *= light;
	
	#ifdef TEX_GLOW
		frag_color.rgb += texture(material.glow_texture, vert_texcoord).rgb;
	#endif
}
)=="