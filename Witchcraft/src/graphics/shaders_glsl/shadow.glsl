R"==(
#begin vertex shader

layout (location = 0) in vec3 in_position;

uniform mat4 mvp;

#include inc/skeletal_animation.glsl

void main()
{
#ifdef SKELETAL_ANIMATION
	gl_Position = mvp * vec4(ApplyBoneTransform(vec4(in_position, 1)).xyz, 1);
#else
	gl_Position = mvp * vec4(in_position, 1);
#endif // SKELETAL_ANIMATION
}

#begin fragment shader

layout (location = 0) out vec2 frag_color;

#define BIAS 0.00333

void main()
{
//	gl_FragDepth = gl_FragCoord.z;
//	gl_FragDepth += gl_FrontFacing ? BIAS : 0.0;
}

)=="