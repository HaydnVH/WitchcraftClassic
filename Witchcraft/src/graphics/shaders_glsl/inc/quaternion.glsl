R"==(
vec3 quaternion_rotate(vec4 quat, vec3 rhs)
{
	vec3 qvec = quat.xyz;
	vec3 uv = cross(qvec, rhs);

	return rhs + (2.0f * ((uv * quat.w) + cross(qvec, uv)));
}
)=="