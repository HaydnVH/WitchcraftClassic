R"==(
#begin vertex shader
#ifdef SKELETAL_ANIMATION

layout (location = 4) in ivec4 in_bones;
layout (location = 5) in vec4 in_weights;

//struct BoneTransform
//{
//	vec4 rotation;
//	vec4 position;
//};

layout (std140) uniform SKELETON
{
	mat4 bones[256];
//	BoneTransform bones[256];
};

vec4 ApplyBoneTransform(vec4 val)
{
	return
	((bones[in_bones[0]] * val) * in_weights[0]) +
	((bones[in_bones[1]] * val) * in_weights[1]) +
	((bones[in_bones[2]] * val) * in_weights[2]) +
	((bones[in_bones[3]] * val) * in_weights[3]);
}

#endif // SKELETAL_ANIMATION
#end // vertex
)=="