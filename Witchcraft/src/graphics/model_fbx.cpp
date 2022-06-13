#include "model.h"

using namespace std;

#include <fbxsdk.h>

#include "sys/printlog.h"

using namespace vmath;

namespace {

FbxManager* fbxman = nullptr;
int sdk_majorver = 0, sdk_minorver = 0, sdk_revision = 0;

struct VertexInformation
{
	vec3 position;
	vec4 color;
	vec3 normal;
	vec2 texcoord;
	vec3 tangent;
	vec3 binormal;
	float bitangent_sign;
	int bones[4];
	vec4 bone_weights;
};

struct PackedVertexInformation
{
	// Note: we can theoretically pack vertices even tighter than this,
	// by storing a qtangent instead of normal+tangent,
	// and using shorts for position (with shading in position.w)
	vec3 position;
	half u, v;
	char nx, ny, nz, shading;
	char tx, ty, tz, bs;
	unsigned char bone[4];
	unsigned char weight[4];

	// Determine if two vertices are (functionally) equivelant.
	bool operator == (const PackedVertexInformation& rhs)
	{
		// Positions are floats, so we require a fair amount of precision here.
		// If two positions are within 0.01mm of each other,
		// we consider them close enough to be the same location.
		vec3 posdiff = position - rhs.position;
		if (abs(posdiff.x) > 0.00001f)
			return false;
		if (abs(posdiff.y) > 0.00001f)
			return false;
		if (abs(posdiff.z) > 0.00001f)
			return false;
		
		// Texture co-ordinates are half-floats, and need less precision than positions.
		vec2 uvdiff = vec2(u, v) - vec2(rhs.u, rhs.v);
		if (abs(uvdiff.s) > 0.0001f)
			return false;
		if (abs(uvdiff.t) > 0.0001f)
			return false;

		// Normals, tangents, and additional data along for the ride are all stored as 8-bit integers,
		// therefore we can easily perform exact (in)equality comparisons.
		if (nx != rhs.nx || ny != rhs.ny || nz != rhs.nz || shading != rhs.shading ||
			tx != rhs.tx || ty != rhs.ty || tz != rhs.tz || bs != rhs.bs)
			return false;

		// Bones and weights are also 8-bit integers, so we can test for (in)equality here as well.
		if (bone[0] != rhs.bone[0] || bone[1] != rhs.bone[1] || bone[2] != rhs.bone[2] || bone[3] != rhs.bone[3] ||
			weight[0] != rhs.weight[0] || weight[1] != rhs.weight[1] || weight[2] != rhs.weight[2] || weight[3] != rhs.weight[3])
			return false;

		// If none of the above tests returned false, then the two vertices are identical,
		// or at the very least, close enough to justify welding them together.
		return true;
	}
};

void MergeVertex(vector<PackedVertexInformation>& vertices, vector<int>& indices, size_t first, size_t second)
{	
	// Get the index at the rear of the list
	size_t back = vertices.size() - 1;

	// Move the rearmost vertex to the spot of the 'second' vertex.
	vertices[second] = vertices[back];

	// Delete the rearmost vertex
	vertices.pop_back();

	// Loop through the indices...
	for (size_t i = 0; i < indices.size(); ++i)
	{
		// If we used to point to 'second', we should now point to 'first'.
		if (indices[i] == (int)second)
			indices[i] = (int)first;

		// If we used to point to 'back', we should now point to 'second' (where we moved 'back' to).
		if (indices[i] == (int)back)
			indices[i] = (int)second;
	}
}

void RemoveDuplicateVertices(vector<PackedVertexInformation>& vertices, vector<int>& indices, size_t offset)
{
	for (size_t i = offset; i < vertices.size(); ++i)
	{
		for (size_t j = i + 1; j < vertices.size(); ++j)
		{
			if (vertices[i] == vertices[j])
			{
				MergeVertex(vertices, indices, i, j);
				--j;
			}
		}
	}
}

struct MeshGeometry
{
	vector<PackedVertexInformation> vertices;
	vector<int> indices;

	string material;
	int start;
	int count;
	bool has_skinning;
};

struct AnimationKey
{
	float timestamp;
	vec3 position;
	quat rotation;
	vec3 scale;
};

// Determines whether or not "mid_val" is a linear interpolation between start_val and end_val according to the indicated times.
// This is used to remove redundant keyframes.
template <typename T>
bool isLerp(float start_time, float mid_time, float end_time, T start_val, T mid_val, T end_val)
{
	// Move t0 to 0 and adjust other times accordingly.
	end_time -= start_time;
	mid_time -= start_time;
	start_time = 0.0f;

	// Move t2 to 1 and adjust t1 accordingly.
	float timescale = 1 / end_time;
	mid_time *= timescale;
	end_time *= timescale;

	// Right now, start_time should be 0, end_time should be 1, and mid_time should be somewhere in between.
	// We can now use mid_time to calculate a mid_val and compare it to the actual mid_val.
	T calculated_mid = T::lerp(start_val, end_val, mid_time);
	return T::close_enough(mid_val, calculated_mid);
}

struct AnimChannelInformation
{
	string bone_name;
	vector<AnimationKey> keys;
};

struct AnimationInformation
{
	float duration;
	vector<AnimChannelInformation> anim_channels;
};

struct BoneInfo
{
	string name;
	mat4 inverse_global_bind_pose;
	mat4 local_bind_pose;
	mat4 local_default_pose;
	FbxAMatrix local_pose_difference;
	int parent_index;
};

} // namespace <anon>

void LoadFbxSkeleton(FbxNode* node, vector<BoneInfo>& skeleton, unordered_map<FixedString<32>, int32_t>& bone_map, int parent_index);
void LoadFbxMesh(FbxNode* node, vector<MeshGeometry>& geom, vector<BoneInfo>& skeleton, unordered_map<FixedString<32>, int32_t>& bone_map);
void LoadFbxCollisionMesh(FbxNode* node, vector<PackedVertexInformation>& vertices, vector<int>& indices);

bool Model::LoadFBX(const char* filename, const char* password)
{
	if (fbxman == nullptr)
	{
		fbxman = FbxManager::Create();
		// TODO: Error checking!

		//Create an IOSettings object. This object holds all import/export settings.
		FbxIOSettings* ios = FbxIOSettings::Create(fbxman, IOSROOT);
		fbxman->SetIOSettings(ios);

		//Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		fbxman->LoadPluginsDirectory(lPath.Buffer());

		// Get the FBX version of the SDK
		FbxManager::GetFileFormatVersion(sdk_majorver, sdk_minorver, sdk_revision);
	}

	Clear();

	FbxImporter* importer = FbxImporter::Create(fbxman, "");

	bool import_status = importer->Initialize(filename, -1, fbxman->GetIOSettings());

	// Get the FBX version of the file.
	int file_majorver = 0, file_minorver = 0, file_revision = 0;
	importer->GetFileVersion(file_majorver, file_minorver, file_revision);

	if (import_status == false)
	{
		plog::error("FbxImporter::Initialize() failed:\n");
		plog::errmore("%s\n", importer->GetStatus().GetErrorString());

		if (importer->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			plog::error("FBX file format version for this FBX SDK is %i.%i.%i\n", sdk_majorver, sdk_minorver, sdk_revision);
			plog::errmore("FBX file format version for '%s' is %i.%i.%i\n", filename, file_majorver, file_minorver, file_revision);
		}

		return false;
	}

	plog::info("FBX SDK version %i.%i.%i\n", sdk_majorver, sdk_minorver, sdk_revision);

	if (importer->IsFBX())
	{
		plog::info("FBX file version %i.%i.%i\n", file_majorver, file_minorver, file_revision);

		// Set the import states.  By defauly, these are always true.
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
		fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* scene = FbxScene::Create(fbxman, "My Scene");
	if (scene == nullptr)
	{
		plog::error("Failed to create FBX Scene.\n");
		return false;
	}
	// TODO: Moar error checking!

	// Import the scene.
	import_status = importer->Import(scene);

	if (import_status == false && importer->GetStatus().GetCode() == FbxStatus::ePasswordError)
	{
		if (password == nullptr)
		{
			plog::error("Could not load FBX file because it is password protected.\n");
		}
		else
		{
			fbxman->GetIOSettings()->SetStringProp(IMP_FBX_PASSWORD, password);
			fbxman->GetIOSettings()->SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

			import_status = importer->Import(scene);

			if (import_status == false && importer->GetStatus().GetCode() == FbxStatus::ePasswordError)
			{
				plog::error("Incorrect password.\n");
			}
		}
	}

	// Destroy the importer.
	importer->Destroy();
	importer = nullptr;

	// Make sure we successfully imported the file.
	if (import_status == false)
	{
		plog::error("Failed to import file '%s' using FBX SDK.\n", filename);
		scene->Destroy();
		return false;
	}

	// Triangulate every mesh in the model.
	FbxGeometryConverter converter(fbxman);
	converter.Triangulate(scene, true);

	FbxNode* root = scene->GetRootNode();

	plog::info("Found %i poses.\n", scene->GetPoseCount());

	for (int posei = 0; posei < scene->GetPoseCount(); ++posei)
	{
		FbxPose* pose = scene->GetPose(posei);
		if (pose->IsValidBindPose(root))
			{ plog::info("Pose [%i] is a valid bind pose.\n", posei); }
		else
			{ plog::info("Pose [%i] is NOT a valid bind pose!\n", posei); }
	}

	// Load the skeleton hierarchy.
	vector<BoneInfo> boneinfo;
	LoadFbxSkeleton(root, boneinfo, skeleton.bone_map, -1);

	// Load the geometry data.
	vector<MeshGeometry> geom;
	LoadFbxMesh(root, geom, boneinfo, skeleton.bone_map);

	// Use the inverse bind pose to calculate the bones' local matrices.
	for (size_t i = 0; i < boneinfo.size(); ++i)
	{
		boneinfo[i].local_bind_pose = boneinfo[i].inverse_global_bind_pose.inverted();
		if (boneinfo[i].parent_index >= 0)
		{
			boneinfo[i].local_bind_pose = boneinfo[boneinfo[i].parent_index].inverse_global_bind_pose * boneinfo[i].local_bind_pose;
		}

		// Calculate the pose difference matrix so we can repair the animation matrices.
		// For models where the default pose is the bind pose, this will end up being an identity matrix.
		mat4 posediff = boneinfo[i].local_default_pose.inverted() * boneinfo[i].local_bind_pose;
		for (int i = 0; i < 16; ++i)
			{ ((double*)boneinfo[i].local_pose_difference)[i] = (double)posediff.data[i]; }
	}

	// Load the collision mesh.
	vector<PackedVertexInformation> pmesh_vertices;
	vector<int> pmesh_indices;
	LoadFbxCollisionMesh(root, pmesh_vertices, pmesh_indices);
	RemoveDuplicateVertices(pmesh_vertices, pmesh_indices, 0);

	
	// Now that we have the geometry for each mesh in the model, we need to merge meshes which share materials.
	for (size_t i = 0; i < geom.size(); ++i)
	{
		for (size_t j = i+1; j < geom.size(); ++j)
		{
			if (geom[i].material == geom[j].material)
			{
				int vertex_offset = (int)geom[i].vertices.size();
				// Merge the two lists of vertices.
				geom[i].vertices.insert(geom[i].vertices.end(), geom[j].vertices.begin(), geom[j].vertices.end());
				// Correct the second list of indices.
				for (auto& index : geom[j].indices) { index += vertex_offset; }
				// Merge the two lists of indices.
				geom[i].indices.insert(geom[i].indices.end(), geom[j].indices.begin(), geom[j].indices.end());
				geom[i].count += geom[j].count;
				// Erase the information from the second list.
				geom.erase(geom.begin() + j);
				--j;
			}
		}
	}

	// This is where we'd like to do post-load processing, such as vertex cache optimization.

	// Here we assemble the vertices and indices from each mesh together.
	vector<PackedVertexInformation> vertices;
	vector<int> indices;
	int running_index = 0;
	int running_count = 0;
	bool has_skinning = false;
	for (size_t i = 0; i < geom.size(); ++i)
	{
		RemoveDuplicateVertices(geom[i].vertices, geom[i].indices, 0);
		vertices.insert(vertices.end(), geom[i].vertices.begin(), geom[i].vertices.end());
		for (auto& index : geom[i].indices) { index += running_index; }
		indices.insert(indices.end(), geom[i].indices.begin(), geom[i].indices.end());
		geom[i].start = running_count;

		running_index += (int)geom[i].vertices.size();
		running_count += geom[i].count;

		if (geom[i].has_skinning)
			has_skinning = true;
	}

	// Arrange the data in our vertex buffer format
	this->geom.vertex_format = VF_POSITION | VF_SURFACE;
	if (has_skinning) this->geom.vertex_format |= VF_SKIN;
	this->geom.num_vertices = (uint32_t)vertices.size();
	size_t vertex_buffer_size = this->geom.num_vertices * calc_bytes_per_vertex(this->geom.vertex_format);

	this->geom.num_indices = (uint32_t)indices.size();
	if (this->geom.num_vertices > USHRT_MAX)
		{ this->geom.index_size = 4; }
	else if (this->geom.num_vertices > UCHAR_MAX)
		{ this->geom.index_size = 2; }
	else
		{ this->geom.index_size = 1; }
	size_t index_buffer_size = (size_t)(this->geom.num_indices * this->geom.index_size);

	this->geom.buffer_size = (uint32_t)(vertex_buffer_size + index_buffer_size);
	this->geom.buffer_ptr = new char[this->geom.buffer_size];

	void* running_ptr = this->geom.buffer_ptr;
	this->geom.positions_ptr = (VertexPosition*)running_ptr; running_ptr = (void*)(this->geom.positions_ptr + this->geom.num_vertices);
	this->geom.surface_ptr = (VertexSurface*)running_ptr; running_ptr = (void*)(this->geom.surface_ptr + this->geom.num_vertices);
	if (this->geom.vertex_format & VF_SKIN)
		{ this->geom.skin_ptr = (VertexSkin*)running_ptr; running_ptr = (void*)(this->geom.skin_ptr + this->geom.num_vertices); }
	this->geom.indices_ptr = running_ptr;


	// Copy our vertex data into our new buffer.
	for (int32_t i = 0; i < this->geom.num_vertices; ++i)
	{
		this->geom.positions_ptr[i].x = vertices[i].position.x;
		this->geom.positions_ptr[i].y = vertices[i].position.y;
		this->geom.positions_ptr[i].z = vertices[i].position.z;

		this->geom.surface_ptr[i].s = vertices[i].u;
		this->geom.surface_ptr[i].t = vertices[i].v;
		this->geom.surface_ptr[i].nx = vertices[i].nx;
		this->geom.surface_ptr[i].ny = vertices[i].ny;
		this->geom.surface_ptr[i].nz = vertices[i].nz;
		this->geom.surface_ptr[i].shading = vertices[i].shading;
		this->geom.surface_ptr[i].tx = vertices[i].tx;
		this->geom.surface_ptr[i].ty = vertices[i].ty;
		this->geom.surface_ptr[i].tz = vertices[i].tz;
		this->geom.surface_ptr[i].bs = vertices[i].bs;

		if (has_skinning)
		{
			for (int j = 0; j < 4; ++j)
			{
				this->geom.skin_ptr[i].bone[j] = vertices[i].bone[j];
				this->geom.skin_ptr[i].weight[j] = vertices[i].weight[j];
			}
		}
	}

	for (int32_t i = 0; i < this->geom.num_indices; ++i)
	{
		switch (this->geom.index_size)
		{
		case 4:
			((uint32_t*)this->geom.indices_ptr)[i] = (uint32_t)indices[i]; break;
		case 2:
			((uint16_t*)this->geom.indices_ptr)[i] = (uint32_t)indices[i]; break;
		case 1:
			((uint8_t*)this->geom.indices_ptr)[i] = (uint8_t)indices[i]; break;
		}
	}

	// Calculate the size of the persistant buffer.
	persistant_buffer_size = 0;

	persistant_buffer_size += (uint32_t)(this->meshes.meshsize * geom.size());

	persistant_buffer_size += (uint32_t)(this->skeleton.bonesize * boneinfo.size());

	if (pmesh_vertices.size() > 0)
	{
		persistant_buffer_size += (uint32_t)(sizeof(VertexPosition) * pmesh_vertices.size());
		persistant_buffer_size += (uint32_t)(sizeof(int32_t) * pmesh_indices.size());
	}
	else
	{
		persistant_buffer_size += (uint32_t)sizeof(SimpleShapeCollider);
	}

	int animstack_count = scene->GetSrcObjectCount<FbxAnimStack>();
	persistant_buffer_size += (uint32_t)sizeof(AnimationClip) * animstack_count;

	// Allocate memory for the persistant buffer.
	persistant_buffer = new char[persistant_buffer_size];
	running_ptr = persistant_buffer;

	// Get our pointers from within the persistant buffer.
	{
		meshes.count = (int32_t)geom.size();
		meshes.start = (int32_t*)running_ptr; running_ptr = (void*)(meshes.start + meshes.count);
		meshes.primcount = (int32_t*)running_ptr; running_ptr = (void*)(meshes.primcount + meshes.count);
		meshes.material_id = (FixedString<32>*)running_ptr; running_ptr = (void*)(meshes.material_id + meshes.count);
		meshes.material_ptr = (Material**)running_ptr; running_ptr = (void*)(meshes.material_ptr + meshes.count);

		// Get the per-mesh information
		for (size_t i = 0; i < meshes.count; ++i)
		{
			meshes.start[i] = geom[i].start;
			meshes.primcount[i] = geom[i].count;
			meshes.material_id[i] = geom[i].material.c_str();
			meshes.material_ptr[i] = nullptr;
		}

		skeleton.num_bones = (int32_t)boneinfo.size();
		if (skeleton.num_bones > 0)
		{
			skeleton.bone_name = (FixedString<32>*)running_ptr; running_ptr = (void*)(skeleton.bone_name + skeleton.num_bones);
			skeleton.inv_bind_pose = (mat4*)running_ptr; running_ptr = (void*)(skeleton.inv_bind_pose + skeleton.num_bones);
			skeleton.to_parent = (mat4*)running_ptr; running_ptr = (void*)(skeleton.to_parent + skeleton.num_bones);
			skeleton.parent_index = (int32_t*)running_ptr; running_ptr = (void*)(skeleton.parent_index + skeleton.num_bones);
			skeleton.collider = (btCollisionShape * *)running_ptr; running_ptr = (void*)(skeleton.collider + skeleton.num_bones);
			skeleton.collider_offset = (vec3*)running_ptr; running_ptr = (void*)(skeleton.collider_offset + skeleton.num_bones);
			skeleton.collider_flags = (uint32_t*)running_ptr; running_ptr = (void*)(skeleton.collider_flags + skeleton.num_bones);

			// Get the skeleton information
			for (uint32_t i = 0; i < boneinfo.size(); ++i)
			{
				skeleton.bone_name[i] = boneinfo[i].name.c_str();
				skeleton.inv_bind_pose[i] = boneinfo[i].inverse_global_bind_pose;
				skeleton.to_parent[i] = boneinfo[i].local_bind_pose;
				skeleton.parent_index[i] = boneinfo[i].parent_index;
				skeleton.collider[i] = nullptr;
				skeleton.collider_offset[i] = VEC3_ZERO;
				skeleton.collider_flags[i] = 0;
			}
		}

		// Copy the collision mesh into the model
		if (pmesh_vertices.size() > 0)
		{
			collision.type = COLLIDER_CONCAVEMESH;
			collision.num_vertices = (int32_t)pmesh_vertices.size();
			collision.num_indices = (int32_t)pmesh_indices.size();
			collision.vertices_ptr = (VertexPosition*)running_ptr; running_ptr = (void*)(collision.vertices_ptr + collision.num_vertices);
			collision.indices_ptr = (int32_t*)running_ptr; running_ptr = (void*)(collision.indices_ptr + collision.num_indices);

			for (size_t i = 0; i < pmesh_vertices.size(); ++i)
				{ collision.vertices_ptr[i] = pmesh_vertices[i].position; }

			for (size_t i = 0; i < pmesh_indices.size(); ++i)
				{ collision.indices_ptr[i] = pmesh_indices[i]; }
		}
		else
			{ collision.type = COLLIDER_NULL; }
	}


	// And we're done here (for now)! print some information about the model.
	plog::info("Found %i meshes,\n", geom.size());
	plog::infomore("containing %i vertices and %i indices (%i triangles).\n", vertices.size(), indices.size(), indices.size() / 3);
	for (size_t i = 0; i < geom.size(); ++i)
	{
		plog::infomore("  [%i] '%s' (%i index count).\n", i, geom[i].material, geom[i].count);
	}
	plog::info("Found %i bones,\n", skeleton.num_bones);
	for (size_t i = 0; i < skeleton.num_bones; ++i)
	{
		plog::infomore("  [%i] '%s'", i, skeleton.bone_name[i]);
		if (skeleton.parent_index[i] >= 0 && skeleton.parent_index[i] < skeleton.num_bones && i != skeleton.parent_index[i])
			plog::print(", child of [%i] '%s'.\n", skeleton.parent_index[i], skeleton.bone_name[skeleton.parent_index[i]]);
		else
			plog::print(".\n");
	}

	// Get the animations in the file.
	plog::info("Found %i animation stack(s),\n", animstack_count);
	animations.count = animstack_count;
	animations.clip = (AnimationClip*)running_ptr; running_ptr = (void*)(animations.clip + animations.count);

	for (int animstack_i = 0; animstack_i < scene->GetSrcObjectCount<FbxAnimStack>(); ++animstack_i)
	{
		FbxAnimStack* animstack = scene->GetSrcObject<FbxAnimStack>(animstack_i);
		plog::info(" Animation stack name: '%s'.\n", animstack->GetName());

		// Construct the animation clip in-place and grab our pointer to it.
		AnimationClip* clip = new (&animations.clip[animstack_i]) AnimationClip();

		int layer_count = animstack->GetMemberCount<FbxAnimLayer>();
		if (layer_count != 1)
		{
			plog::errmore("  Excpected 1 animation layer, found %i.\n", layer_count);
			continue;
		}

		clip->name = animstack->GetName();

		FbxAnimLayer* layer = animstack->GetSrcObject<FbxAnimLayer>(0);
		int curvenode_count = layer->GetSrcObjectCount<FbxAnimCurveNode>();

		clip->channels.resize(skeleton.num_bones);
		
		for (int curvenode_i = 0; curvenode_i < curvenode_count; ++curvenode_i)
		{
			FbxAnimCurveNode* curvenode = layer->GetSrcObject<FbxAnimCurveNode>(curvenode_i);
			int curve_count = curvenode->GetCurveCount(0);
			if (curve_count != 1)
			{
			//	plog::error(ERRMORE, "  Expected 1 curve, found ", curve_count, "\n");
				continue;
			}

			FbxAnimCurve* curve = curvenode->GetCurve(0);
			FbxAnimCurveDef::EInterpolationType itype = curve->KeyGetInterpolation(0);

			// Find every property of this node affected by this animation
			int property_count = curvenode->GetDstPropertyCount();
			for (int property_i = 0; property_i < property_count; ++property_i)
			{
				FbxProperty prop = curvenode->GetDstProperty(property_i);
				FbxNode* node = (FbxNode*)prop.GetFbxObject();

				if (skeleton.bone_map.count(node->GetName()) == 0)
				{
					plog::errmore("    Found animation data for bone '%s' which does not exist in the skeleton.\n", node->GetName());
					continue;
				}
				size_t bone_index = skeleton.bone_map[node->GetName()];
				if (clip->channels[bone_index].isEmpty())
					{ clip->num_used_channels++; }
				clip->channels[bone_index].bone_name = node->GetName();
				AnimationChannel& channel = clip->channels[bone_index];
				FbxString propname = prop.GetName();
				
				if (node->LclTranslation.IsValid() && propname.Compare(node->LclTranslation.GetName()) == 0)
				{
					// Add translation keyframes
					int key_count = curve->KeyGetCount();
					for (int key_i = 0; key_i < key_count; ++key_i)
					{
						FbxTime keytime = curve->KeyGetTime(key_i);
						FbxAMatrix mat; mat.SetT(node->EvaluateLocalTranslation(keytime));
//						mat *= boneinfo[bone_index].local_pose_difference;

						FbxVector4 local_translation = mat.GetT();
						vec3 pos = {};
						for (int i = 0; i < 3; ++i)
							{ pos.data[i] = (float)local_translation.mData[i]; }
						channel.position_keys.push_back((float)keytime.GetSecondDouble(), pos);
					}

					// Clean up translation keyframes
					if (channel.position_keys.size() > 0)
					{
						// Get the timestamp of the first keyframe.
			//			float first_time = channel.position_keys.get<0>(0);
			//			if (first_time != 0.0f)
			//			{
			//				// If the timestamp of the first keyframe is not at 0, we need to correct all of our timestamps.
			//				for (int key_i = 0; key_i < channel.position_keys.size(); ++key_i)
			//				{
			//					channel.position_keys.get<0>(key_i) -= first_time;
			//				}
			//			}

						if (channel.position_keys.back<0>() > clip->duration)
							clip->duration = channel.position_keys.back<0>();
					}

				}
				else if (node->LclRotation.IsValid() && propname.Compare(node->LclRotation.GetName()) == 0)
				{
					// Add rotation keyframes
					int key_count = curve->KeyGetCount();
					for (int key_i = 0; key_i < key_count; ++key_i)
					{
						FbxTime keytime = curve->KeyGetTime(key_i);
//						FbxVector4 local_rotation = node->EvaluateLocalRotation(keytime);
						FbxAMatrix mat; mat.SetR(node->EvaluateLocalRotation(keytime));
//						mat *= boneinfo[bone_index].local_pose_difference;

						FbxQuaternion local_rot = mat.GetQ();
						quat rot = QUAT_IDENTITY;
						for (int i = 0; i < 4; ++i)
							{ rot.data[i] = (float)local_rot.mData[i]; }
//						quat rot = quat::euler({ (float)local_rotation.mData[0], (float)local_rotation.mData[1], (float)local_rotation.mData[2] });
						channel.rotation_keys.push_back((float)keytime.GetSecondDouble(), rot);
					}

					// Clean up rotation keyframes
					if (channel.rotation_keys.size() > 0)
					{
			//			// Get the timestamp of the first keyframe.
			//			float first_time = channel.rotation_keys.get<0>(0);
			//			if (first_time != 0.0f)
			//			{
			//				// If the timestamp of the first keyframe is not at 0, we need to correct all of our timestamps.
			//				for (int key_i = 0; key_i < channel.rotation_keys.size(); ++key_i)
			//				{
			//					channel.rotation_keys.get<0>(key_i) -= first_time;
			//				}
			//			}

						if (channel.rotation_keys.back<0>() > clip->duration)
							clip->duration = channel.rotation_keys.back<0>();
					}
					
				}
				else if (node->LclScaling.IsValid() && propname.Compare(node->LclScaling.GetName()) == 0)
				{
					// Add scaling keyframes
					int key_count = curve->KeyGetCount();
					for (int key_i = 0; key_i < key_count; ++key_i)
					{
						FbxTime keytime = curve->KeyGetTime(key_i);
						FbxAMatrix mat; mat.SetS(node->EvaluateLocalScaling(keytime));
//						mat *= boneinfo[bone_index].local_pose_difference;

						FbxVector4 local_scaling = mat.GetS();
						vec3 scl = {};
						for (int i = 0; i < 3; ++i)
							{ scl.data[i] = (float)local_scaling.mData[i]; }
						channel.scale_keys.push_back((float)keytime.GetSecondDouble(), scl);
					}

					// Clean up scaling keyframes
					if (channel.scale_keys.size() > 0)
					{
						// Get the timestamp of the first keyframe.
			//			float first_time = channel.scale_keys.get<0>(0);
			//			if (first_time != 0.0f)
			//			{
			//				// If the timestamp of the first keyframe is not at 0, we need to correct all of our timestamps.
			//				for (int key_i = 0; key_i < channel.scale_keys.size(); ++key_i)
			//				{
			//					channel.scale_keys.get<0>(key_i) -= first_time;
			//				}
			//			}

						if (channel.scale_keys.back<0>() > clip->duration)
							clip->duration = channel.scale_keys.back<0>();
					}

				}
				else
				{
					plog::errmore("    Found invalid property name '%s'.\n", propname.Buffer());
					continue;
				}

			} // for each property

		} // for each curvenode

	} // for each animstack

	scene->Destroy();
	return true;
}

void LoadFbxSkeleton(FbxNode* node, vector<BoneInfo>& skeleton, unordered_map<FixedString<32>, int32_t>& bone_map, int parent_index)
{
	int my_index = -1;

	// Get the bone information for this node, if it exists.
	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		BoneInfo bone = {};
		bone.parent_index = parent_index;
		bone.name = node->GetName();
		bone.local_bind_pose = MAT4_IDENTITY;
		bone.inverse_global_bind_pose = MAT4_IDENTITY;
		bone.local_default_pose = MAT4_IDENTITY;
		bone.local_pose_difference.SetIdentity();

		// Get the local default pose matrix.
		// For some models, this will be equal to the local bind pose.
		// If this were always the case, our lives would be a lot easier.
		// We need this for when it isn't the case, so we can fix the animation matrices.
		FbxAMatrix local_transform = node->EvaluateLocalTransform();
		for (int i = 0; i < 16; ++i)
			{ bone.local_default_pose.data[i] = (float)((double*)local_transform)[i]; }

		my_index = (int)skeleton.size();
		skeleton.push_back(bone);

		bone_map[bone.name.c_str()] = my_index;
	}

	// Load data from child nodes.
	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		LoadFbxSkeleton(node->GetChild(i), skeleton, bone_map, my_index);
	}

	return;
}

struct IndexWeightPair
{
	size_t index;
	float weight;
};

struct VertexBoneWeightInformation
{
	vector<IndexWeightPair> list;
};

void LoadFbxMesh(FbxNode* node, vector<MeshGeometry>& geom, vector<BoneInfo>& skeleton, unordered_map<FixedString<32>, int32_t>& bone_map)
{
	int vertid = 0;
	MeshGeometry meshinfo = {};

	// Get mesh data from this node (if there is any)
	FbxMesh* mesh = node->GetMesh();

	// Check to make sure this isn't supposed to be a collision mesh.
	string nodename = node->GetName();
	if (nodename.find("COLLISION") == string::npos && mesh != nullptr)
	{
		vector<VertexBoneWeightInformation> vertex_bone_weights;
		vertex_bone_weights.resize(mesh->GetControlPointsCount());

		for (int deformer_index = 0; deformer_index < mesh->GetDeformerCount(); ++deformer_index)
		{
			FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformer_index, FbxDeformer::eSkin);
			if (!skin)
				continue;

			meshinfo.has_skinning = true;

			for (int cluster_index = 0; cluster_index < skin->GetClusterCount(); ++cluster_index)
			{
				FbxCluster* cluster = skin->GetCluster(cluster_index);
				string joint_name = cluster->GetLink()->GetName();
				if (bone_map.count(joint_name.c_str()) == 0)
				{
					plog::error("Mesh is deformed by joint which is not present in the skeleton.\n");
					plog::errmore("Joint name: '%s'.\n", joint_name);
					continue;
				}
				size_t joint_index = bone_map[joint_name.c_str()];

				// Associate each joint with the control points it affects.
				for (int i = 0; i < cluster->GetControlPointIndicesCount(); ++i)
				{
					IndexWeightPair pair = {};
					pair.index = joint_index;
					pair.weight = (float)(cluster->GetControlPointWeights()[i]);
					int control_point_index = cluster->GetControlPointIndices()[i];
					if (control_point_index >= vertex_bone_weights.size())
						vertex_bone_weights.resize(control_point_index + 1);
					vertex_bone_weights[control_point_index].list.push_back(pair);
				}

				// Get the matrices for this cluster.
				FbxAMatrix transformMatrix; cluster->GetTransformMatrix(transformMatrix);
				FbxAMatrix transformLinkMatrix; cluster->GetTransformLinkMatrix(transformLinkMatrix);
				FbxAMatrix globalInverseBindPose = transformLinkMatrix.Inverse() * transformMatrix;

				for (int i = 0; i < 16; ++i)
				{
					skeleton[joint_index].inverse_global_bind_pose.data[i] = (float)((double*)globalInverseBindPose)[i];
				}

			} // for each cluster
		} // for each deformer

		// With vertex skinning information available, we need to normalize the lists so no more than 4 bones affect each vertex.
		for (size_t i = 0; i < vertex_bone_weights.size(); ++i)
		{
			// If this vertex is affected by less than 4 joints, we need to expand the list and fill the new entries with zeroes.
			if (vertex_bone_weights[i].list.size() < 4)
			{
				vertex_bone_weights[i].list.resize(4, { 0, 0.0f });
			}
			// If this vertex is affected by more than 4 joints, we need to shrink the list and adjust weights accordingly.
			else if (vertex_bone_weights[i].list.size() > 4)
			{
				sort(vertex_bone_weights[i].list.begin(), vertex_bone_weights[i].list.end(), [](const IndexWeightPair& lhs, const IndexWeightPair& rhs)
					{ return lhs.weight > rhs.weight; });

				float weight_sum = 0.0f;
				for (size_t j = 0; j < 4; ++j)
					{ weight_sum += vertex_bone_weights[i].list[j].weight; }
				for (int j = 0; j < 4; ++j)
					{ vertex_bone_weights[i].list[j].weight /= weight_sum; }
				vertex_bone_weights[i].list.resize(4);
			}
		}

		mesh->GenerateNormals(false, true);
		mesh->GenerateTangentsDataForAllUVSets(false);

		int triangle_count = mesh->GetPolygonCount();
		for (int t = 0; t < triangle_count; ++t)
		{

			for (int v = 0; v < 3; ++v)
			{
				VertexInformation vert = {};

				// Get position (control point) for this vertex
				int ctrlpoint = mesh->GetPolygonVertex(t, v);
				FbxVector4 pos = mesh->GetControlPointAt(ctrlpoint);
				vert.position.x = (float)pos.mData[0];
				vert.position.y = (float)pos.mData[1];
				vert.position.z = (float)pos.mData[2];


				// Colors
				FbxColor col = {};
				if (mesh->GetElementVertexColorCount() > 0)
				{
					FbxGeometryElementVertexColor* vc = mesh->GetElementVertexColor(0);

					if (vc->GetMappingMode() == FbxGeometryElement::eByControlPoint)
					{
						if (vc->GetReferenceMode() == FbxGeometryElement::eDirect)
							col = vc->GetDirectArray().GetAt(ctrlpoint);
						else if (vc->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = vc->GetIndexArray().GetAt(ctrlpoint);
							col = vc->GetDirectArray().GetAt(id);
						}
						else {} // Invalid vertex color reference mode.
					}
					else if (vc->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						if (vc->GetReferenceMode() == FbxGeometryElement::eDirect)
							col = vc->GetDirectArray().GetAt(vertid);
						else if (vc->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = vc->GetIndexArray().GetAt(vertid);
							col = vc->GetDirectArray().GetAt(id);
						}
						else
						{
						}	// Invalid vertex color reference mode.
					}
					else {}	// Invalid vertex color mapping mode.
				}
				vert.color.r = (float)col.mRed;
				vert.color.g = (float)col.mGreen;
				vert.color.b = (float)col.mBlue;
				vert.color.a = (float)col.mAlpha;

				// Texture Coordinates
				FbxVector2 uvs = {};
				if (mesh->GetElementUVCount() > 0)
				{
					FbxGeometryElementUV* uv = mesh->GetElementUV(0);

					if (uv->GetMappingMode() == FbxGeometryElement::eByControlPoint)
					{
						if (uv->GetReferenceMode() == FbxGeometryElement::eDirect)
							uvs = uv->GetDirectArray().GetAt(ctrlpoint);
						else if (uv->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = uv->GetIndexArray().GetAt(ctrlpoint);
							uvs = uv->GetDirectArray().GetAt(id);
						}
						else {} // Invalid vertex texcoord reference mode.
					}
					else if (uv->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						int uvindex = mesh->GetTextureUVIndex(t, v);
						if (uv->GetReferenceMode() == FbxGeometryElement::eDirect ||
							uv->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							uvs = uv->GetDirectArray().GetAt(uvindex);
						}
						else {} // Invalid vertex texcoord reference mode.
					}
					else {} // Invalid vertex texcoord mapping mode.
				}
				vert.texcoord.s = (float)uvs.mData[0];
				vert.texcoord.t = (float)uvs.mData[1];

				// Normals
				FbxVector4 nrm = {};
				if (mesh->GetElementNormalCount() > 0)
				{
					FbxGeometryElementNormal* ptr = mesh->GetElementNormal(0);

					if (ptr->GetMappingMode() == FbxGeometryElement::eByControlPoint)
					{
						if (ptr->GetReferenceMode() == FbxGeometryElement::eDirect)
							nrm = ptr->GetDirectArray().GetAt(ctrlpoint);
						else if (ptr->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = ptr->GetIndexArray().GetAt(ctrlpoint);
							nrm = ptr->GetDirectArray().GetAt(id);
						}
						else {} // Invalid vertex texcoord reference mode.
					}
					else if (ptr->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						if (ptr->GetReferenceMode() == FbxGeometryElement::eDirect)
							nrm = ptr->GetDirectArray().GetAt(vertid);
						else if (ptr->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = ptr->GetIndexArray().GetAt(vertid);
							nrm = ptr->GetDirectArray().GetAt(id);
						}
						else {} // Invalid vertex normal reference mode.
					}
					else {} // Invalid vertex normal mapping mode.
				}
				vert.normal.x = (float)nrm.mData[0];
				vert.normal.y = (float)nrm.mData[1];
				vert.normal.z = (float)nrm.mData[2];
				vert.normal.normalize();

				// Tangents
				FbxVector4 tan = {};
				if (mesh->GetElementTangentCount() > 0)
				{
					FbxGeometryElementTangent* ptr = mesh->GetElementTangent(0);

					if (ptr->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						if (ptr->GetReferenceMode() == FbxGeometryElement::eDirect)
							tan = ptr->GetDirectArray().GetAt(vertid);
						else if (ptr->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = ptr->GetIndexArray().GetAt(vertid);
							tan = ptr->GetDirectArray().GetAt(id);
						}
						else {} // Invalid vertex normal reference mode.
					}
					else {} // Invalid vertex normal mapping mode.
				}
				vert.tangent.x = (float)tan.mData[0];
				vert.tangent.y = (float)tan.mData[1];
				vert.tangent.z = (float)tan.mData[2];
				vert.tangent.normalize();

				// Binormals
				FbxVector4 bin = {};
				if (mesh->GetElementBinormalCount() > 0)
				{
					FbxGeometryElementBinormal* ptr = mesh->GetElementBinormal(0);

					if (ptr->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						if (ptr->GetReferenceMode() == FbxGeometryElement::eDirect)
							bin = ptr->GetDirectArray().GetAt(vertid);
						else if (ptr->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						{
							int id = ptr->GetIndexArray().GetAt(vertid);
							bin = ptr->GetDirectArray().GetAt(id);
						}
						else {} // Invalid vertex normal reference mode.
					}
					else {} // Invalid vertex normal mapping mode.
				}
				vert.binormal.x = (float)bin.mData[0];
				vert.binormal.y = (float)bin.mData[1];
				vert.binormal.z = (float)bin.mData[2];
				vert.binormal.normalize();

				// Once we've finished loading the vertex data from the FBX,
				// We need to process it a little so it'll fit in our vertex buffers.
				PackedVertexInformation pvert = {};
				pvert.position = vert.position;
				pvert.u = vert.texcoord.s;
				pvert.v = vert.texcoord.t;
				pvert.nx = (char)(vert.normal.x * (float)(SCHAR_MAX));
				pvert.ny = (char)(vert.normal.y * (float)(SCHAR_MAX));
				pvert.nz = (char)(vert.normal.z * (float)(SCHAR_MAX));
				pvert.tx = (char)(vert.tangent.x * (float)(SCHAR_MAX));
				pvert.ty = (char)(vert.tangent.y * (float)(SCHAR_MAX));
				pvert.tz = (char)(vert.tangent.z * (float)(SCHAR_MAX));

				// Here we calculate vertex shading.
				// Since vertex colors are usually only used to fake shading or ambient occlusion
				// in objects which use tiling/generic textures (such as architecture),
				// we really only need a single greyscale vertex color.
				// This conveniently fits into the w component of the normal, saving memory.
				float shade = (vert.color.r + vert.color.g + vert.color.b) / 3.0f;
				pvert.shading = (char)(shade * (float)SCHAR_MAX);

				// Here we calculate the bitangent sign.
				// We do this by producing the cross product of the normal and the tangent, as in the shader.
				// This vector should either have the same sign on all 3 axes as the FBX-loaded binormal (bitangent sign == +1),
				// or the opposite sign on all 3 axes (bitangent sign == -1).
				// We do need to account for the possibility that one or more axis could be zero,
				// which is the only case where the sign of the axis could be the same if the vectors are actually opposite.
				vec3 calced_bitangent = vec3::cross(vert.normal, vert.tangent);
				if (calced_bitangent.x != 0.0f)
				{
					if (calced_bitangent.x < 0 && vert.binormal.x > 0)
						pvert.bs = SCHAR_MIN;
					else
						pvert.bs = SCHAR_MAX;
				}
				else if (calced_bitangent.y != 0.0f)
				{
					if (calced_bitangent.y < 0 && vert.binormal.y > 0)
						pvert.bs = SCHAR_MIN;
					else
						pvert.bs = SCHAR_MAX;
				}
				else if (calced_bitangent.z != 0.0f)
				{
					if (calced_bitangent.z < 0 && vert.binormal.z > 0)
						pvert.bs = SCHAR_MIN;
					else
						pvert.bs = SCHAR_MAX;
				}

				// Skinning information
				if (meshinfo.has_skinning)
				{
					for (int i = 0; i < 4; ++i)
					{
						pvert.bone[i] = (uint8_t)vertex_bone_weights[ctrlpoint].list[i].index;
						pvert.weight[i] = (uint8_t)(vertex_bone_weights[ctrlpoint].list[i].weight * (float)UCHAR_MAX);
					}
				}

				meshinfo.vertices.push_back(pvert);
				meshinfo.indices.push_back(vertid);

				vertid++;
			} // vertex loop
		} // triangle loop

		meshinfo.count = vertid;
		meshinfo.material = mesh->GetNode()->GetName();

		if (mesh->GetElementMaterialCount() > 0)
		{
			FbxSurfaceMaterial* mat = mesh->GetNode()->GetMaterial(0);
			meshinfo.material = mat->GetName();
		}

		geom.push_back(meshinfo);
	} // if mesh

	  // Load data from child nodes.
	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		LoadFbxMesh(node->GetChild(i), geom, skeleton, bone_map);
	}
	
	return;
}


void LoadFbxCollisionMesh(FbxNode* node, vector<PackedVertexInformation>& vertices, vector<int>& indices)
{
	// Get mesh data from this node (if there is any)
	FbxMesh* mesh = node->GetMesh();

	int vertid = (int)vertices.size();

	// Check to make sure this is supposed to be a collision mesh.
	string nodename = node->GetName();
	if (nodename.find("COLLISION") != string::npos && mesh != nullptr)
	{
		int triangle_count = mesh->GetPolygonCount();
		for (int t = 0; t < triangle_count; ++t)
		{
			for (int v = 0; v < 3; ++v)
			{
				PackedVertexInformation vert = {};

				// Get position (control point) for this vertex
				int ctrlpoint = mesh->GetPolygonVertex(t, v);
				FbxVector4 pos = mesh->GetControlPointAt(ctrlpoint);
				vert.position.x = (float)pos.mData[0];
				vert.position.y = (float)pos.mData[1];
				vert.position.z = (float)pos.mData[2];

				vertices.push_back(vert);
				indices.push_back(vertid);

				vertid++;

			} // for each vertex

		} // for each triangle

	} // if (mesh)

	for (int i = 0; i < node->GetChildCount(); ++i)
		{ LoadFbxCollisionMesh(node->GetChild(i), vertices, indices); }
}