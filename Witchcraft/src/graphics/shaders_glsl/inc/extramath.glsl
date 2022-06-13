R"==(

float linstep(float v, float min, float max)
{
	return clamp((v-min) / (max-min), 0, 1);
}

float median(vec3 vals)
{
	return max(min(vals.r, vals.g), min(max(vals.r, vals.g), vals.b));
}

//vec4 trilerp(vec4 a, vec4 b, vec4 c, float t, float mid)
//{
//	return mix(mix(a, b, linstep(0, mid, t)), mix(b, c, linstep(mid, 1, t)), t);
//}

)=="