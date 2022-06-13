#ifndef HVH_WC_SCENE_TRANSFORM_H
#define HVH_WC_SCENE_TRANSFORM_H

#include "entity.h"
#include "math/vmath.h"

/*
The 'transform' component stores an entity's position, orientation, and scale in the world.
It also keeps track of these states for the previous frame, allowing interpolation between frames,
and handles creating the World matrix to be sent to the GPU.
*/

namespace entity {
namespace transform {

	/* Adds a transform to the entity and sets its initial state. */
	/* Default parameters (zero, identity, one) results in an identity matrix. */
	void add(ID id, vmath::vec3 pos = vmath::VEC3_ZERO, vmath::quat rot = vmath::QUAT_IDENTITY, vmath::vec3 scale = vmath::VEC3_ONE);
	/* Removes the transform from an entity, if it has one. */
	void remove(ID id);
	/* Returns whether or not the entity has a transform component. */
	bool has(ID id);

	/* Sets an entity's position. */
	void setPos(ID id, vmath::vec3 new_pos);
	/* Moves an entity (adds delta_pos to its existing position). */
	void move(ID id, vmath::vec3 delta_pos);
	/* Gets an entity's position. */
	vmath::vec3 getPos(ID id);
	/* Causes an entity's position to 'snap' to its current position, so it doesn't interpolate with where it used to be. */
	/* This should be used if a smooth transition is not desired. */
	void snapPos(ID id);
	/* Creates an interpolation between an entity's old position and its current position. */
	vmath::vec3 interpolatePos(ID id, float interpolation);

	/* Sets an entity's rotation. */
	void setRot(ID id, vmath::quat new_rot);
	/* Rotates an entity (multiplies its existing rotation by delta_rot). */
	void rotate(ID id, vmath::quat delta_rot);
	/* Gets an enttiy's rotation. */
	vmath::quat getRot(ID id);
	/* Causes an entity's rotation to 'snap' to its current rotation, so it doesn't interpolate with its old orientation. */
	/* This should be used if a smooth transition is not desired. */
	void snapRot(ID id);
	/* Creates an interpolation between an entity's old rotation and its current rotation. */
	vmath::quat interpolateRot(ID id, float interpolation);

	/* Sets an entity's scale. */
	void setScale(ID id, vmath::vec3 new_scale);
	/* Gets an entity's scale. */
	vmath::vec3 getScale(ID id);
	/* Causes an entity's scale to 'snap' to its current scale, so it doesn't interpolatate with its old scale. */
	void snapScale(ID id);
	/* Creates an interpolation between an entity's old scale and its current scale. */
	vmath::vec3 interpolateScale(ID id, float interpolation);

	/* Gets the calculated matrix for an entity. */
	/* This should not be used within the logical update! */
	/* Matrices are calculated in the display update, and this function is only useful after that happens. */
	vmath::mat4 getMatrix(ID id);
	/* Applies a matrix transformation to the entity's calculated matrix. */
	/* (this matrix * transform) */
	void applyPreTransform(ID id, vmath::mat4 transform);
	/* Applies a matrix transformation to the entity's calculated matrix. */
	/* (transform * this matrix) */
	void applyPostTransform(ID id, vmath::mat4 transform);

	/* Sets the previous transform for every entry to be the current transform. */
	/* This should be called only at the beginning of the logical update. */
	void Flip();
	/* Uses the current and previous transform state to calculate the Model matrix for each entry. */
	/* This should be called only at the beginning of the display update. */
	void CalcMatrices(float interpolation);

	/* Initializes the Lua interface for the transform component. */
	/* This should be called exactly once during program initialization. */
	void InitLua();

	/* Returns the number of bytes used by a given number of entity's transforms; Used for memory allocations. */
	constexpr size_t size_of(size_t count)
	{
		return count * (sizeof(vmath::vec3) + sizeof(vmath::quat) + sizeof(vmath::vec3) +
						sizeof(vmath::vec3) + sizeof(vmath::quat) + sizeof(vmath::vec3) +
						sizeof(vmath::mat4));
	}

	/* Takes a pointer to memory allocated by the scene and allocates sections of it for our purposes. */
	/* Returns a pointer to the memory immediately after what we used, which can be sent to the next component's allocation. */
	void* Allocate(void* memory, size_t capacity);

	/* Destroys everything. */
	void Clear();


}} // namespace component::transform


#endif // HVH_WC_SCENE_TRANSFORM_H/