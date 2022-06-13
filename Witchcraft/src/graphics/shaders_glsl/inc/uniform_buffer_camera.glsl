R"==(
layout (std140) uniform CAMERA
{
	mat4 view;
	mat4 proj;
	mat4 projview;
	mat4 invproj;
	vec4 eye_pos;
	vec4 eye_dir;
	vec2 screen_size;
	float aspect_ratio;
	float frame_interpolation;
	uint logical_frame_counter;
	uint display_frame_counter;
};
)=="