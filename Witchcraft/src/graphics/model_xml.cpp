#include "model.h"
#include <pugixml.hpp>
using namespace pugi;

#include <sstream>
using namespace std;
using namespace vmath;

const char* Model::LoadXML(istream& file)
{
	Clear();

	// Load in the file's contents.
	stringstream ss;
	ss << file.rdbuf();
	string file_contents = ss.str();

	// Parse the file.
	xml_document doc;
	xml_parse_result parse_result = doc.load_buffer_inplace((void*)file_contents.data(), file_contents.size());
	if (!parse_result)
		{ return "Error parsing file."; }

	xml_node root = doc.child("model");
	if (!root)
		{ return "Could not find 'model' root node."; }

	// Geometry section
	xml_node geometry_node = root.child("geometry");
	if (geometry_node)
	{
		// Number of vertices in the model
		xml_node numvertices_node = geometry_node.child("num_vertices");
		if (!numvertices_node)
			{ return "Model geometry does not have a 'num_vertices' node."; }
		geom.num_vertices = numvertices_node.text().as_uint();

		// Number of triangles (and therefore indices) in the model
		xml_node numtriangles_node = geometry_node.child("num_indices");
		if (!numtriangles_node)
			{ return "Model geometry does not have a 'num_indices' node."; }
		geom.num_indices = numtriangles_node.text().as_uint();

		// Vertex data
		xml_node vertexdata_node = geometry_node.child("vertex_data");
		if (!vertexdata_node)
			{ return "Model geometry does not have a 'vertex_data' node."; }
		{
			// First we have to find which vertex attributes are present so we know how much memory to allocate per-vertex.
			xml_node positions_node = vertexdata_node.child("positions");
			if (positions_node)
				{ geom.vertex_format |= VF_POSITION; }

			xml_node texcoords_node = vertexdata_node.child("texcoords");
			xml_node colors_node = vertexdata_node.child("colors");
			xml_node shading_node = vertexdata_node.child("shading");
			xml_node normals_node = vertexdata_node.child("normals");
			xml_node tangents_node = vertexdata_node.child("tangents");
			if (texcoords_node || normals_node || tangents_node)
				{ geom.vertex_format |= VF_SURFACE; }

			xml_node skin_node = vertexdata_node.child("skin");
			if (skin_node)
				{ geom.vertex_format |= VF_SKIN; }

			// Now that we know how many large each vertex is, we can allocate space to store them.
			uint32_t bytes_per_vertex = calc_bytes_per_vertex(geom.vertex_format);
			uint32_t vertex_buffer_size = bytes_per_vertex * geom.num_vertices;

			if (geom.num_vertices > USHRT_MAX) geom.index_size = 4;
			else if (geom.num_vertices > UCHAR_MAX) geom.index_size = 2;
			else geom.index_size = 1;
			uint32_t index_buffer_size = geom.index_size * geom.num_indices;

			geom.buffer_size = vertex_buffer_size + index_buffer_size;
			geom.buffer_ptr = new char[geom.buffer_size];

			//geom.positions_ptr = (VertexPosition*)geom.buffer_ptr;
			geom.surface_ptr = (VertexSurface*)(geom.positions_ptr + geom.num_vertices);
			geom.skin_ptr = (VertexSkin*)(geom.surface_ptr + geom.num_vertices);
			geom.indices_ptr = (void*)(geom.skin_ptr + geom.num_vertices);

			size_t offset = 0;

			if (positions_node)
			{
				geom.positions_ptr = (VertexPosition*)((char*)geom.buffer_ptr + offset);
				offset += sizeof(VertexPosition) * geom.num_vertices;

				ss.str(positions_node.text().as_string());

				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					VertexPosition pos = {};
					ss >> pos.x;
					ss >> pos.y;
					ss >> pos.z;
					geom.positions_ptr[i] = pos;
				}
			}

			if (geom.vertex_format & VF_SURFACE)
			{
				geom.surface_ptr = (VertexSurface*)((char*)geom.buffer_ptr + offset);
				offset += sizeof(VertexSurface) * geom.num_vertices;

				if (texcoords_node)
				{
					ss.str(texcoords_node.text().as_string());
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						float f;
						ss >> f; geom.surface_ptr[i].s = f;
						ss >> f; geom.surface_ptr[i].t = -f; // NOTE: Inverting texcoord.t here! Maybe should do this elsewhere?
					}
				}
				else
				{
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						geom.surface_ptr[i].s = 0.0f;
						geom.surface_ptr[i].t = 0.0f;
					}
				}

				if (shading_node)
				{
					ss.str(shading_node.text().as_string());
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						int tmp;
						ss >> tmp;
						geom.surface_ptr[i].shading = (int8_t)(tmp);
					}
				}
				else if (colors_node)
				{
					ss.str(colors_node.text().as_string());
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						vec3 col = {};
						int tmp;
						ss >> tmp; col.r = (float)tmp / (float)UCHAR_MAX;
						ss >> tmp; col.g = (float)tmp / (float)UCHAR_MAX;
						ss >> tmp; col.b = (float)tmp / (float)UCHAR_MAX;
						ss >> tmp; // col.a
						float shade = (col.r + col.g + col.b) / 3.0f;
						geom.surface_ptr[i].shading = (int8_t)(shade * (float)SCHAR_MAX);
					}
				}
				else
				{
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						geom.surface_ptr[i].shading = SCHAR_MAX;
					}
				}

				if (normals_node)
				{
					ss.str(normals_node.text().as_string());
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						int tmp;
						ss >> tmp; geom.surface_ptr[i].nx = (int8_t)tmp;
						ss >> tmp; geom.surface_ptr[i].ny = (int8_t)tmp;
						ss >> tmp; geom.surface_ptr[i].nz = (int8_t)tmp;
					}
				}
				else
				{
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						geom.surface_ptr[i].nx = 0;
						geom.surface_ptr[i].ny = 0;
						geom.surface_ptr[i].nz = 0;
					}
				}

				if (tangents_node)
				{
					ss.str(tangents_node.text().as_string());
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						int tmp;
						ss >> tmp; geom.surface_ptr[i].tx = (int8_t)tmp;
						ss >> tmp; geom.surface_ptr[i].ty = (int8_t)tmp;
						ss >> tmp; geom.surface_ptr[i].tz = (int8_t)tmp;
						ss >> tmp; geom.surface_ptr[i].bs = (int8_t)tmp;
					}
				}
				else
				{
					for (int32_t i = 0; i < geom.num_vertices; ++i)
					{
						geom.surface_ptr[i].tx = 0;
						geom.surface_ptr[i].ty = 0;
						geom.surface_ptr[i].tz = 0;
						geom.surface_ptr[i].bs = 0;
					}
				}
			} // if (vertex_format & VF_SURFACE)

			if (skin_node)
			{
				ss.str(skin_node.text().as_string());
				geom.skin_ptr = (VertexSkin*)((char*)geom.buffer_ptr + offset);
				offset += sizeof(VertexSkin) * geom.num_vertices;
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					VertexSkin skin;
					int tmp;
					ss >> tmp; skin.bone[0] = (uint8_t)tmp;
					ss >> tmp; skin.bone[1] = (uint8_t)tmp;
					ss >> tmp; skin.bone[2] = (uint8_t)tmp;
					ss >> tmp; skin.bone[3] = (uint8_t)tmp;

					ss >> tmp; skin.weight[0] = (uint8_t)tmp;
					ss >> tmp; skin.weight[1] = (uint8_t)tmp;
					ss >> tmp; skin.weight[2] = (uint8_t)tmp;
					ss >> tmp; skin.weight[3] = (uint8_t)tmp;

					// Here we double check to make sure the skin weights add up to 1.0 (255)
					int weight_total = 0;
					int greatest_weight = 0;
					int greatest_weight_num = 0;
					for (int j = 0; j < 4; ++j)
					{
						weight_total += skin.weight[j];
						if (skin.weight[j] > greatest_weight)
						{
							greatest_weight = skin.weight[j];
							greatest_weight_num = j;
						}
					}
					if (weight_total < 255)
					{
						skin.weight[greatest_weight_num] += (255 - weight_total);
					}

					geom.skin_ptr[i] = skin;
				}
			} // if (skin_node)

			geom.indices_ptr = (void*)((char*)geom.buffer_ptr + offset);
		}

		// Index data
		xml_node triangledata_node = geometry_node.child("triangle_data");
		if (!triangledata_node)
			{ return "Model geometry does not have a 'triangle_data' node."; }
		{
			ss.str(triangledata_node.text().as_string());
			switch (geom.index_size)
			{
			case 1:
				for (int32_t i = 0; i < geom.num_indices; ++i)
					{ int tmp; ss >> tmp; ((uint8_t*)geom.indices_ptr)[i] = (uint8_t)tmp; }
				break;
			case 2:
				for (int32_t i = 0; i < geom.num_indices; ++i)
					{ ss >> ((uint16_t*)geom.indices_ptr)[i]; }
				break;
			case 4:
				for (int32_t i = 0; i < geom.num_indices; ++i)
					{ ss >> ((uint32_t*)geom.indices_ptr)[i]; }
				break;
			}
		}
	}

	xml_node import_transform_node = root.child("transform");
	if (import_transform_node)
	{
		xml_node matrix_node = import_transform_node.child("matrix");
		if (matrix_node)
		{
			ss.clear(); ss.str(matrix_node.text().as_string());
			for (int i = 0; i < 16; ++i)
			{
				ss >> import_transform.data[i];
			}
		}
		else
		{
			xml_node translation_node = import_transform_node.child("translation");
			if (translation_node)
			{
				vec3 translation = VEC3_ZERO;

				ss.clear(); ss.str(translation_node.text().as_string());
				for (int i = 0; i < 3; ++i)
				{
					ss >> translation.data[i];
				}

				import_transform *= mat4::translation(translation);
			}

			xml_node rotation_node = import_transform_node.child("rotation");
			if (rotation_node)
			{
				vec3 euler_angles = VEC3_ZERO;
				
				ss.clear(); ss.str(rotation_node.text().as_string());
				for (int i = 0; i < 3; ++i)
					{ ss >> euler_angles.data[i]; }

				quat rotation = quat::euler(euler_angles * TO_RADIANS);
				import_transform *= mat4::rotation(rotation);
			}

			xml_node scale_node = import_transform_node.child("scale");
			if (scale_node)
			{
				vec3 scale = VEC3_ONE;

				ss.clear(); ss.str(scale_node.text().as_string());
				for (int i = 0; i < 3; ++i)
					{ ss >> scale.data[i]; }

				import_transform *= mat4::scale(scale);
			}
		}
	}

	// We have to determine the size of the persistant buffer.
	xml_node meshes_node = root.child("meshes");
	xml_node skeleton_node = root.child("skeleton");
	xml_node collision_node = root.child("physics");
	xml_node animations_node = root.child("animations");

	if (meshes_node)
	{
		xml_node num_meshes_node = meshes_node.child("num_meshes");
		if (!num_meshes_node)
			{ return "Model does not indicate the number of meshes."; }
		meshes.count = num_meshes_node.text().as_uint();
		if (meshes.count <= 0)
			{ return "Model must have at least one mesh."; }

		persistant_buffer_size += meshes.meshsize * meshes.count;
	}

	if (skeleton_node)
	{
		xml_node num_bones_node = skeleton_node.child("num_bones");
		if (!num_bones_node)
			{ return "Skeleton node is present but does not indicate the number of bones."; }
		skeleton.num_bones = num_bones_node.text().as_int();

		persistant_buffer_size += skeleton.bonesize * skeleton.num_bones;
	}

	if (collision_node)
	{
		// Collision node is either a list of simple shapes, or a triangle mesh; we have to determine the collider type first.
		xml_node collidertype_node = collision_node.child("collider_type");
		if (!collidertype_node) { return "Physics does not indicate its collider type."; }
		const char* collidertype_str = collidertype_node.text().as_string();

		if (COLLIDER_TYPE_STR_TO_ENUM.count(collidertype_str) == 0)
			{ return "Invalid collider type."; }

		collision.type = COLLIDER_TYPE_STR_TO_ENUM.at(collidertype_str);
		switch (collision.type)
		{
		case COLLIDER_SIMPLE:
			collision.num_simple_shapes = 1;
			persistant_buffer_size += sizeof(SimpleShapeCollider);
			break;
		case COLLIDER_COMPOUND:
			// We currently don't have actual support for compound colliders.  We should do something about that...
			break;
		case COLLIDER_CONVEXMESH:
		case COLLIDER_CONCAVEMESH:
			{
				xml_node vertsnode = collision_node.child("vertices");
				if (!vertsnode) { return "Mesh collider does not have a list of vertices."; }
				xml_attribute count_attrib = vertsnode.attribute("count");
				if (!count_attrib) { return "Mesh collider does not indicate vertex count."; }
				collision.num_vertices = count_attrib.as_int();
				persistant_buffer_size += sizeof(VertexPosition) * collision.num_vertices;

				xml_node indsnode = collision_node.child("indices");
				if (!indsnode) { return "Mesh collider does not have a list of indices."; }
				count_attrib = indsnode.attribute("count");
				if (!count_attrib) { return "Mesh collider does not indicate index count."; }
				collision.num_indices = count_attrib.as_int();
				persistant_buffer_size += sizeof(int32_t) * collision.num_indices;
			}
			break;
		}
	}

	if (animations_node)
	{
		for (xml_node anim_node = animations_node.child("animation"); anim_node; anim_node = anim_node.next_sibling("animation"))
			{ animations.count++; }

		persistant_buffer_size += sizeof(AnimationClip) * animations.count;
	}

	// With the persistant buffer size in hand, we can finally allocate the persistant buffer.
	persistant_buffer = new char[persistant_buffer_size];

	// This is the pointer that we'll increment as we allocate portions of the buffer to each list in the model.
	void* running_ptr = persistant_buffer;

	// Parse the Skeleton section
	if (skeleton_node)
	{
		xml_node bones_array_node = skeleton_node.child("bones_array");
		if (!bones_array_node)
			{ return "Skeleton node is present but does not contain a node for the bones array."; }

		skeleton.bone_name = (FixedString<32>*)running_ptr;
		skeleton.inv_bind_pose = (mat4*)(skeleton.bone_name + skeleton.num_bones);
		skeleton.to_parent = (mat4*)(skeleton.inv_bind_pose + skeleton.num_bones);
		skeleton.parent_index = (int32_t*)(skeleton.to_parent + skeleton.num_bones);
		skeleton.collider = (btCollisionShape**)(skeleton.parent_index + skeleton.num_bones);
		skeleton.collider_offset = (vec3*)(skeleton.collider + skeleton.num_bones);
		skeleton.collider_flags = (uint32_t*)(skeleton.collider_offset + skeleton.num_bones);
		running_ptr = (void*)(skeleton.collider_flags + skeleton.num_bones);

		for (xml_node bone_node = bones_array_node.child("bone"); bone_node; bone_node = bone_node.next_sibling("bone"))
		{
			xml_attribute bone_index_attribute = bone_node.attribute("index");
			if (!bone_index_attribute)
				{ return "Bone node does not have an 'index' attribute."; }
			int bone_index = bone_index_attribute.as_int();

			if (bone_index < 0 || bone_index >= skeleton.num_bones)
				{ return "Bone index out of bounds.\n"; }

			xml_node bone_name_node = bone_node.child("name");
			if (!bone_name_node)
				{ return "Bone node does not have a 'name' node."; }
			const char* bone_name = bone_name_node.text().as_string();
			skeleton.bone_name[bone_index] = bone_name;
			skeleton.bone_map[bone_name] = bone_index;

			xml_node bone_invbindpose_node = bone_node.child("offset_matrix");
			mat4 matrix = MAT4_IDENTITY;
			if (bone_invbindpose_node)
			{
				ss.clear(); ss.str(bone_invbindpose_node.text().as_string());
				for (int i = 0; i < 16; ++i)
					{ ss >> matrix.data[i]; }
			}
			else
			{
				// error: bone doesn't have an inverse bind pose matrix
			}
			skeleton.inv_bind_pose[bone_index] = matrix;

			xml_node bone_toparent_node = bone_node.child("local_matrix");
			matrix = MAT4_IDENTITY;
			if (bone_toparent_node)
			{
				ss.clear(); ss.str(bone_toparent_node.text().as_string());
				for (int i = 0; i < 16; ++i)
					{ ss >> matrix.data[i]; }
			}
			else {}
			skeleton.to_parent[bone_index] = matrix;

			xml_node bone_parent_node = bone_node.child("parent");
			int parent = -1;
			if (bone_parent_node)
			{
				parent = bone_parent_node.text().as_int();
			}
			skeleton.parent_index[bone_index] = parent;

			skeleton.collider_flags[bone_index] = 0;

			xml_node bone_collider_node = bone_node.child("collider");
			if (bone_collider_node)
			{
				SimpleShapeCollider collider;
				string shape_type; vec3 shape_dimensions;
				ss.clear(); ss.str(bone_collider_node.text().as_string());
				ss >> shape_type;
				for (int i = 0; i < 3; ++i) { ss >> shape_dimensions.data[i]; }
				if (COLLIDER_TYPE_STR_TO_ENUM.count(shape_type) > 0)
				{
					collider = { COLLIDER_TYPE_STR_TO_ENUM.at(shape_type), shape_dimensions };
					skeleton.collider[bone_index] = (btCollisionShape*)collider.makeCollider();
				}
				
			//	if (bone_node.child("default_dynamic"))
			//		{ skeleton.collider_flags[bone_index] |= RD_DEFAULTDYNAMIC; }

				xml_node collider_offset_node = bone_node.child("collider_offset");
				if (collider_offset_node)
				{
					ss.clear(); ss.str(collider_offset_node.text().as_string());
					vec3 offset; for (int i = 0; i < 3; ++i) { ss >> offset.data[i]; }
					skeleton.collider_offset[bone_index] = offset;
				}
				else
					{ skeleton.collider_offset[bone_index] = VEC3_ZERO; }
			}
			else
				{ skeleton.collider[bone_index] = nullptr; }
		}
		
		// With all of the bone data loaded, now we need to transform the model's vertices into the skeleton's bind pose.
		// TODO: Move this into the FBX import stage!
		/*
		vector<mat4> bind_pose(skeleton.num_bones);
		vector<quat> bind_rots(skeleton.num_bones);
		for (int32_t i = 0; i < skeleton.num_bones; ++i)
		{
			bind_pose[i] = skeleton.to_parent[i].inverted();
			//if (skeleton.parent_index[i] > 0 && skeleton.parent_index[i] < skeleton.num_bones)
			//	{ bind_pose[i] = bind_pose[skeleton.parent_index[i]] * bind_pose[i]; }
		}
		for (int32_t i = 0; i < skeleton.num_bones; ++i)
		{
			//bind_pose[i] *= skeleton.inv_bind_pose[i];
		}

		for (int32_t i = 0; i < geom.num_vertices; ++i)
		{
			vec4 pos = { geom.positions_ptr[i].to_vec3(), 1.0f };
			vec4 nrm = { geom.surface_ptr[i].normal_to_vec3(), 0.0f };
			vec4 tng = { geom.surface_ptr[i].tangent_to_vec3(), 0.0f };

			pos=((bind_pose[geom.skin_ptr[i].bone[0]] * pos) * geom.skin_ptr[i].weight[0]) +
				((bind_pose[geom.skin_ptr[i].bone[1]] * pos) * geom.skin_ptr[i].weight[1]) +
				((bind_pose[geom.skin_ptr[i].bone[2]] * pos) * geom.skin_ptr[i].weight[2]) +
				((bind_pose[geom.skin_ptr[i].bone[3]] * pos) * geom.skin_ptr[i].weight[3]);

			nrm=((bind_pose[geom.skin_ptr[i].bone[0]] * nrm) * geom.skin_ptr[i].weight[0]) +
				((bind_pose[geom.skin_ptr[i].bone[1]] * nrm) * geom.skin_ptr[i].weight[1]) +
				((bind_pose[geom.skin_ptr[i].bone[2]] * nrm) * geom.skin_ptr[i].weight[2]) +
				((bind_pose[geom.skin_ptr[i].bone[3]] * nrm) * geom.skin_ptr[i].weight[3]);

			tng=((bind_pose[geom.skin_ptr[i].bone[0]] * tng) * geom.skin_ptr[i].weight[0]) +
				((bind_pose[geom.skin_ptr[i].bone[1]] * tng) * geom.skin_ptr[i].weight[1]) +
				((bind_pose[geom.skin_ptr[i].bone[2]] * tng) * geom.skin_ptr[i].weight[2]) +
				((bind_pose[geom.skin_ptr[i].bone[3]] * tng) * geom.skin_ptr[i].weight[3]);

			geom.positions_ptr[i] = pos.xyz;
			geom.surface_ptr[i].vec3_to_normal(nrm.xyz);
			geom.surface_ptr[i].vec3_to_tangent(tng.xyz);
		}
		*/
		
	}

	// Meshes section
	if (meshes_node)
	{
		meshes.start = (int32_t*)running_ptr;
		meshes.primcount = (int32_t*)(meshes.start + meshes.count);
		meshes.material_id = (FixedString<32>*)(meshes.primcount + meshes.count);
		meshes.material_ptr = (Material * *)(meshes.material_id + meshes.count);
		running_ptr = (void*)(meshes.material_ptr + meshes.count);

		for (xml_node mesh_node = meshes_node.child("mesh"); mesh_node; mesh_node = mesh_node.next_sibling("mesh"))
		{
			uint32_t index = mesh_node.attribute("index").as_uint();
			meshes.material_id[index] = mesh_node.child("material").text().as_string();
			meshes.material_ptr[index] = nullptr;
			meshes.start[index] = mesh_node.child("start").text().as_int();

			xml_node endnode = mesh_node.child("end");
			if (endnode)
			{
				uint32_t end = endnode.text().as_int();
				meshes.primcount[index] = (end - meshes.start[index]) + 1;
			}
			xml_node countnode = mesh_node.child("count");
			if (countnode)
			{
				meshes.primcount[index] = countnode.text().as_int();
			}

			xml_node bonenode = mesh_node.child("bone_attach");
			if (bonenode && geom.skin_ptr)
			{
				const char* bone_name = bonenode.text().as_string();
				if (skeleton.bone_map.count(bone_name) != 0)
				{
					int bone_index = (int)skeleton.bone_map[bone_name];

					for (int i = meshes.start[index]; i < (meshes.start[index] + meshes.primcount[index]); ++i)
					{
						size_t vert = 0xffffffff;
						if (geom.index_size == 1)
							vert = ((uint8_t*)geom.indices_ptr)[i];
						else if (geom.index_size == 2)
							vert = ((uint16_t*)geom.indices_ptr)[i];
						else if (geom.index_size == 4)
							vert = ((uint32_t*)geom.indices_ptr)[i];

						if (vert <= geom.num_vertices)
							geom.skin_ptr[vert] = { (uint8_t)bone_index, 0, 0, 0, 255, 0, 0, 0 };
					}
				}
			}
		}
	}

	// Animations section
	if (animations_node)
	{
		animations.clip = (AnimationClip*)running_ptr;
		running_ptr = (void*)(animations.clip + animations.count);

		uint32_t current_anim_index = 0;
		for (xml_node anim_node = animations_node.child("animation"); anim_node; anim_node = anim_node.next_sibling("animation"))
		{
			// Use placement new to properly handle the constructors.
			// This wont be neccesary as we move away from STL.
			AnimationClip* anim = new (&animations.clip[current_anim_index]) AnimationClip();
			xml_node node;

			xml_attribute name_attrib = anim_node.attribute("name");
			if (!name_attrib) { return "Animation does not have a name."; }
			anim->name = name_attrib.as_string();

			animations.anim_map[anim->name] = anim;

			node = anim_node.child("duration");
			if (!node) { return "Animation does not indicate its duration"; }
			anim->duration = node.text().as_float();

			anim->channels.resize(skeleton.num_bones);

			node = anim_node.child("events");
			if (node)
			{
				for (xml_node event_node = node.child("event"); event_node; event_node = event_node.next_sibling("event"))
				{
					xml_attribute attrib = event_node.attribute("time");
					if (!attrib) { return "Animation event does not have a time."; }
					float time = attrib.as_float();

					const char* event_val = event_node.text().as_string();
					anim->events.push_back(time, event_val);
				}
			}

			node = anim_node.child("channels");
			if (!node) { return "Animation does not have channels."; }
			for (xml_node channel_node = node.child("bone_channel"); channel_node; channel_node = channel_node.next_sibling("bone_channel"))
			{
				xml_node subnode = channel_node.child("bone_name");
				if (!subnode) { return "Animation bone channel does not have a name."; }
				const char* bone_name = subnode.text().as_string();

				if (skeleton.bone_map.count(bone_name) == 0)
					{ return "Animation exists for a bone which is not present in the skeleton.\n"; }
				size_t channel_index = skeleton.bone_map[bone_name];

				anim->channels[channel_index].bone_name = bone_name;

				subnode = channel_node.child("position_keys");
				if (subnode)
				{
					for (xml_node poskey_node = subnode.child("key"); poskey_node; poskey_node = poskey_node.next_sibling("key"))
					{
						xml_attribute time_attrib = poskey_node.attribute("time");
						if (!time_attrib) { return "Animation position key does not have a 'time'."; }
						float time = time_attrib.as_float();

						ss.clear(); ss.str(poskey_node.text().as_string());
						vec3 val = {}; for (int i = 0; i < 3; ++i) { ss >> val.data[i]; }
						anim->channels[channel_index].position_keys.push_back(time, val);
					}

					// Normalize bone positions?
					if (anim->channels[channel_index].position_keys.size() > 0)
					{
						vec3 firstval = anim->channels[channel_index].position_keys.get<1>(0);
						for (size_t i = 0; i < anim->channels[channel_index].position_keys.size(); ++i)
							{ anim->channels[channel_index].position_keys.get<1>(i) -= firstval; }
					}
				}

				subnode = channel_node.child("rotation_keys");
				if (subnode)
				{
					for (xml_node rotkey_node = subnode.child("key"); rotkey_node; rotkey_node = rotkey_node.next_sibling("key"))
					{
						xml_attribute time_attrib = rotkey_node.attribute("time");
						if (!time_attrib) { return "Animation rotation key does not have a 'time'."; }
						float time = time_attrib.as_float();

						ss.clear(); ss.str(rotkey_node.text().as_string());
						quat val = {}; for (int i = 0; i < 4; ++i) { ss >> val.data[i]; }
						anim->channels[channel_index].rotation_keys.push_back(time, val);
					}
				}

				subnode = channel_node.child("scale_keys");
				if (subnode)
				{
					for (xml_node sckey_node = subnode.child("key"); sckey_node; sckey_node = sckey_node.next_sibling("key"))
					{
						xml_attribute time_attrib = sckey_node.attribute("time");
						if (!time_attrib) { return "Animation scale key does not have a 'time'."; }
						float time = time_attrib.as_float();

						ss.clear(); ss.str(sckey_node.text().as_string());
						vec3 val = {}; for (int i = 0; i < 3; ++i) { ss >> val.data[i]; }
						anim->channels[channel_index].scale_keys.push_back(time, val);
					}
				}
			}

			current_anim_index++;
		}
	}

	// Physics
	if (collision_node)
	{
		xml_node node;
		if (node = collision_node.child("position_offset"))
		{
			ss.clear(); ss.str(node.text().as_string());
			for (int i = 0; i < 3; ++i) { ss >> collision.offset_position.data[i]; }
		}

		if (node = collision_node.child("rotation_offset"))
		{
			ss.clear(); ss.str(node.text().as_string());
			vec3 angles; for (int i = 0; i < 3; ++i) { ss >> angles.data[i]; }
			collision.offset_rotation = quat::euler(angles * TO_RADIANS);
		}

		switch (collision.type)
		{
		case COLLIDER_SIMPLE:
			{
				collision.simple_collider_shapes = (SimpleShapeCollider*)running_ptr;
				running_ptr = (void*)(collision.simple_collider_shapes + collision.num_simple_shapes);

				string shape_type; vec3 shape_dimensions;
				node = collision_node.child("shape");
				if (!node) { return "Collider simple shape missing."; }
				ss.clear(); ss.str(node.text().as_string());
				ss >> shape_type;
				for (int i = 0; i < 3; ++i) { ss >> shape_dimensions.data[i]; }
				if (COLLIDER_TYPE_STR_TO_ENUM.count(shape_type) == 0)
					{ return "Collider simple shape type invalid."; }
				collision.simple_collider_shapes[0] = { COLLIDER_TYPE_STR_TO_ENUM.at(shape_type), shape_dimensions };
			}
			break;
		case COLLIDER_COMPOUND:
	//		{
	//			string shape_type; vec3 shape_dimensions;
	//			node = collision_node.child("shape_list");
	//			if (!node) { return "Collider shape list missing."; }
	//			for (xml_node subshape = node.child("shape"); subshape; subshape = subshape.next_sibling("shape"))
	//			{
	//				ss.clear(); ss.str(subshape.text().as_string());
	//				shape_type.clear(); ss >> shape_type;
	//				for (int i = 0; i < 3; ++i) { ss >> shape_dimensions.data[i]; }
	//				if (COLLIDER_TYPE_STR_TO_ENUM.count(shape_type) == 0)
	//					{ return "Collider simple shape type invalid."; }
	//				compound_collider.push_back({COLLIDER_TYPE_STR_TO_ENUM.at(shape_type), shape_dimensions});
	//			}
	//		}
			break;
		case COLLIDER_CONVEXMESH: // Deliberate overflow; concave and convex mesh use the same data.
		case COLLIDER_CONCAVEMESH:
			{
				collision.vertices_ptr = (VertexPosition*)running_ptr;
				collision.indices_ptr = (int32_t*)(collision.vertices_ptr + collision.num_vertices);
				running_ptr = (void*)(collision.indices_ptr + collision.num_indices);

				node = collision_node.child("vertices");
				ss.clear(); ss.str(node.text().as_string());
				for (int i = 0; i < collision.num_vertices; ++i)
				{
					VertexPosition vert;
					ss >> vert.x; ss >> vert.y; ss >> vert.z;
					collision.vertices_ptr[i] = vert;
				}

				node = collision_node.child("indices");
				ss.clear(); ss.str(node.text().as_string());
				for (int i = 0; i < collision.num_indices; ++i)
					{ int32_t index; ss >> index; collision.indices_ptr[i] = index; }
			}
			break;
		default:
			return "Invalid collider type enum.";

		} // switch (collider_type)

	} // if (collision_node)

	return NULL;
}

const char* Model::SaveXML(ostream& file)
{
	stringstream ss;
	xml_document doc;
	xml_node root = doc.append_child("model");

	xml_node geometry_node = root.append_child("geometry");
	{
		ss.clear(); ss.str("");
		ss << geom.num_vertices;
		geometry_node.append_child("num_vertices").text() = ss.str().c_str();

		ss.clear(); ss.str("");
		ss << geom.num_indices;
		geometry_node.append_child("num_indices").text() = ss.str().c_str();

		xml_node vertexdata_node = geometry_node.append_child("vertex_data");
		{
			if (geom.vertex_format & VF_POSITION)
			{
				ss.clear(); ss.str("");
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					ss << geom.positions_ptr[i].x << ' ' << geom.positions_ptr[i].y << ' ' << geom.positions_ptr[i].z << ' ';
				}
				vertexdata_node.append_child("positions").text() = ss.str().c_str();
			}

			if (geom.vertex_format & VF_SURFACE)
			{
				ss.clear(); ss.str("");
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					ss << (float)geom.surface_ptr[i].s << ' ' << (float)geom.surface_ptr[i].t << ' ';
				}
				vertexdata_node.append_child("texcoords").text() = ss.str().c_str();

				ss.clear(); ss.str("");
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					ss << (int)geom.surface_ptr[i].shading << ' ';
				}
				vertexdata_node.append_child("shading").text() = ss.str().c_str();

				ss.clear(); ss.str("");
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					ss << (int)geom.surface_ptr[i].nx << ' ' << (int)geom.surface_ptr[i].ny << ' ' << (int)geom.surface_ptr[i].nz << ' ';
				}
				vertexdata_node.append_child("normals").text() = ss.str().c_str();

				ss.clear(); ss.str("");
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					ss << (int)geom.surface_ptr[i].tx << ' ' << (int)geom.surface_ptr[i].ty << ' ' << (int)geom.surface_ptr[i].tz << ' ' << (int)geom.surface_ptr[i].bs << ' ';
				}
				vertexdata_node.append_child("tangents").text() = ss.str().c_str();
			}

			if (geom.vertex_format & VF_SKIN)
			{
				ss.clear(); ss.str("");
				for (int32_t i = 0; i < geom.num_vertices; ++i)
				{
					ss << (int)geom.skin_ptr[i].bone[0] << ' ' << (int)geom.skin_ptr[i].bone[1] << ' ' << (int)geom.skin_ptr[i].bone[2] << ' ' << (int)geom.skin_ptr[i].bone[3] << ' ';
					ss << (int)geom.skin_ptr[i].weight[0] << ' ' << (int)geom.skin_ptr[i].weight[1] << ' ' << (int)geom.skin_ptr[i].weight[2] << ' ' << (int)geom.skin_ptr[i].weight[3] << ' ';
				}
				vertexdata_node.append_child("skin").text() = ss.str().c_str();
			}
		}

		ss.clear(); ss.str("");
		switch (geom.index_size)
		{
		case 1:
			for (int32_t i = 0; i < geom.num_indices; ++i)
				{ ss << (int)((uint8_t*)geom.indices_ptr)[i] << ' '; }
			break;
		case 2:
			for (int32_t i = 0; i < geom.num_indices; ++i)
				{ ss << ((uint16_t*)geom.indices_ptr)[i] << ' '; }
			break;
		case 4:
			for (int32_t i = 0; i < geom.num_indices; ++i)
				{ ss << ((uint32_t*)geom.indices_ptr)[i] << ' '; }
			break;
		}
		geometry_node.append_child("triangle_data").text() = ss.str().c_str();
	}

	// Import transform node
	if (import_transform != MAT4_IDENTITY)
	{
		xml_node transform_node = root.append_child("transform");

		ss.clear(); ss.str("");
		for (int i = 0; i < 16; ++i)
			{ ss << import_transform.data[i] << " "; }

		transform_node.append_child("matrix").text() = ss.str().c_str();
	}

	xml_node meshes_node = root.append_child("meshes");
	{
		ss.clear(); ss.str(""); ss << meshes.count;
		meshes_node.append_child("num_meshes").text() = ss.str().c_str();

		for (int32_t i = 0; i < meshes.count; ++i)
		{
			xml_node mesh_node = meshes_node.append_child("mesh");
			ss.clear(); ss.str(""); ss << i;
			mesh_node.append_attribute("index").set_value(i);
			mesh_node.append_child("material").text() = meshes.material_id[i].c_str;
			ss.clear(); ss.str(""); ss << meshes.start[i];
			mesh_node.append_child("start").text() = ss.str().c_str();
			ss.clear(); ss.str(""); ss << meshes.primcount[i];
			mesh_node.append_child("count").text() = ss.str().c_str();
		}
	}

	// Skeleton section, if the model has a skeleton.
	if (skeleton.num_bones > 0)
	{
		xml_node skeleton_node = root.append_child("skeleton");

//		ss.clear(); ss.str("");
//		for (int i = 0; i < 16; ++i)
//			{ ss << inverse_root.data[i] << " "; }
//		skeleton_node.append_child("inverse_root").text() = ss.str().c_str();

		ss.clear(); ss.str(""); ss << skeleton.num_bones;
		skeleton_node.append_child("num_bones").text() = ss.str().c_str();

		xml_node bones_array_node = skeleton_node.append_child("bones_array");

		for (size_t bone_index = 0; bone_index < skeleton.num_bones; ++bone_index)
		{
			xml_node bone_node = bones_array_node.append_child("bone");
			bone_node.append_attribute("index").set_value(bone_index);
			bone_node.append_child("name").text() = skeleton.bone_name[bone_index].c_str;

			ss.clear(); ss.str("");
			for (int i = 0; i < 16; ++i)
				{ ss << skeleton.inv_bind_pose[bone_index].data[i] << " "; }
			bone_node.append_child("offset_matrix").text() = ss.str().c_str();

			ss.clear(); ss.str("");
			for (int i = 0; i < 16; ++i)
				{ ss << skeleton.to_parent[bone_index].data[i] << " "; }
			bone_node.append_child("local_matrix").text() = ss.str().c_str();

			ss.clear(); ss.str(""); ss << skeleton.parent_index[bone_index];
			bone_node.append_child("parent").text() = ss.str().c_str();
		}
	}

	// Animations section, if the model has animations.
	if (animations.count > 0)
	{
		xml_node animations_node = root.append_child("animations");

		for (size_t anim_index = 0; anim_index < animations.count; ++anim_index)
		{
			AnimationClip& anim_clip = animations.clip[anim_index];

			xml_node anim_node = animations_node.append_child("animation");
			anim_node.append_attribute("name").set_value(anim_clip.name.c_str);

			ss.clear(); ss.str(""); ss << anim_clip.duration;
			anim_node.append_child("duration").text() = ss.str().c_str();

			if (anim_clip.events.size() > 0)
			{
				xml_node events_node = anim_node.append_child("events");
				for (size_t event_index = 0; event_index < anim_clip.events.size(); ++event_index)
				{
					xml_node event_node = events_node.append_child("event");
					event_node.append_attribute("time").set_value(anim_clip.events.get<0>(event_index));
					event_node.text() = anim_clip.events.get<1>(event_index).c_str;
				}
			}

			xml_node channels_node = anim_node.append_child("channels");
			for (size_t channel_index = 0; channel_index < anim_clip.channels.size(); ++channel_index)
			{
				AnimationChannel& anim_channel = anim_clip.channels[channel_index];
				if (anim_channel.isEmpty())
					continue;

				xml_node channel_node = channels_node.append_child("bone_channel");
//				if (channel_index > num_bones)
//					channel_node.append_attribute("index").set_value(-1);
//				else
//					channel_node.append_attribute("index").set_value(channel_index);
				channel_node.append_child("bone_name").text() = anim_channel.bone_name.c_str;

				xml_node position_keys_node = channel_node.append_child("position_keys");
				size_t num_position_keys = anim_channel.position_keys.size();
				position_keys_node.append_attribute("count").set_value(num_position_keys);
				for (size_t position_key_index = 0; position_key_index < num_position_keys; ++position_key_index)
				{
					xml_node key_node = position_keys_node.append_child("key");
					key_node.append_attribute("time").set_value(anim_channel.position_keys.get<0>(position_key_index));

					vec3 val = anim_channel.position_keys.get<1>(position_key_index);
					ss.clear(); ss.str(""); for (int i = 0; i < 3; ++i) { ss << val.data[i] << " "; }
					key_node.text() = ss.str().c_str();
				}

				xml_node rotation_keys_node = channel_node.append_child("rotation_keys");
				size_t num_rotation_keys = anim_channel.rotation_keys.size();
				rotation_keys_node.append_attribute("count").set_value(num_rotation_keys);
				for (size_t rotation_key_index = 0; rotation_key_index < num_rotation_keys; ++rotation_key_index)
				{
					xml_node key_node = rotation_keys_node.append_child("key");
					key_node.append_attribute("time").set_value(anim_channel.rotation_keys.get<0>(rotation_key_index));

					quat val = anim_channel.rotation_keys.get<1>(rotation_key_index);
					ss.clear(); ss.str(""); for (int i = 0; i < 4; ++i) { ss << val.data[i] << " "; }
					key_node.text() = ss.str().c_str();
				}

				xml_node scale_keys_node = channel_node.append_child("scale_keys");
				size_t num_scale_keys = anim_channel.scale_keys.size();
				scale_keys_node.append_attribute("count").set_value(num_scale_keys);
				for (size_t scale_key_index = 0; scale_key_index < num_scale_keys; ++scale_key_index)
				{
					xml_node key_node = scale_keys_node.append_child("key");
					key_node.append_attribute("time").set_value(anim_channel.scale_keys.get<0>(scale_key_index));

					vec3 val = anim_channel.scale_keys.get<1>(scale_key_index);
					ss.clear(); ss.str(""); for (int i = 0; i < 3; ++i) { ss << val.data[i] << " "; }
					key_node.text() = ss.str().c_str();
				}
			}
		}
	} // animations

	// Physics stuff
	if (collision.type != COLLIDER_NULL)
	{
		xml_node physics_node = root.append_child("physics");
		ss.clear(); ss.str(""); for (int i = 0; i < 3; ++i) { ss << collision.offset_position.data[i] << " "; }
		physics_node.append_child("position_offset").text() = ss.str().c_str();
		vec3 angles = (collision.offset_rotation.to_euler() * TO_DEGREES);
		ss.clear(); ss.str(""); for (int i = 0; i < 3; ++i) { ss << angles.data[i] << " "; }
		physics_node.append_child("rotation_offset").text() = ss.str().c_str();

		physics_node.append_child("collider_type").text() = ColliderTypeEnumToStr(collision.type);
		switch (collision.type)
		{
		case COLLIDER_SIMPLE:
			ss.clear(); ss.str(""); ss << ColliderTypeEnumToStr(collision.simple_collider_shapes[0].type) << " ";
			ss << collision.simple_collider_shapes[0].args.x << " " << collision.simple_collider_shapes[0].args.y << " " << collision.simple_collider_shapes[0].args.z << " ";
			physics_node.append_child("shape").text() = ss.str().c_str();
			break;
		case COLLIDER_COMPOUND:
	//		{
	//			xml_node listnode = physics_node.append_child("shape_list");
	//			for (size_t i = 0; i < compound_collider.size(); ++i)
	//			{
	//				ss.clear(); ss.str(""); ss << ColliderTypeEnumToStr(compound_collider[i].type) << " ";
	//				ss << compound_collider[i].args.x << " " << compound_collider[i].args.y << " " << compound_collider[i].args.z << " ";
	//				listnode.append_child("shape").text() = ss.str().c_str();
	//			}
	//		}
			break;
		case COLLIDER_CONVEXMESH: // Deliberate overflow; convex and concave meshes use the same data.
		case COLLIDER_CONCAVEMESH:
			{
				xml_node verticesnode = physics_node.append_child("vertices");
				verticesnode.append_attribute("count").set_value(collision.num_vertices);
				ss.clear(); ss.str("");
				for (size_t i = 0; i < collision.num_vertices; ++i)
					{ ss << collision.vertices_ptr[i].x << " " << collision.vertices_ptr[i].y << " " << collision.vertices_ptr[i].z << " "; }
				verticesnode.text() = ss.str().c_str();

				xml_node indicesnode = physics_node.append_child("indices");
				indicesnode.append_attribute("count").set_value(collision.num_indices);
				ss.clear(); ss.str("");
				for (size_t i = 0; i < collision.num_indices; ++i)
					{ ss << collision.indices_ptr[i] << " "; }
				indicesnode.text() = ss.str().c_str();
				break;
			}
		}

	} // physics

	doc.save(file);
	return NULL;
}