-- movement_collision_test.lua

-- Some constants
-- 35 degrees or less is considered "floor"; we shouldn't slide when standing on such a slope.
local flat_ground_threshold = math.cos(35 * math.deg2rad)
-- 60 degrees or above is considered "wall"; this slope is too steep to walk onto.
local too_steep_threshold = math.cos(60 * math.deg2rad)

-- from: The location we're moving from (lying flat against the ground)
-- to: The location we want to move to, according to our direction and velocity.
-- radius: How fat our collider is.
-- height: The distance from the floor to the top of the character's head.
-- downstep:  How far down we're allowed to step before we enter the "falling" state.
-- upstep: How far up we're allowed to step.

function handle_movement_collisions(from, to, radius, height, downstep, upstep)

	-- Get the dimensions of our collider.
	local collider_capsule_height = height - (upstep + (radius * 2))
	local collider_vertical_offset = (height * 0.5) + upstep
	
	-- Move up.
	from = from + (vec3.up * collider_vertical_offset)
	to = to + (vec3.up * collider_vertical_offset)
	
	-- Move horitontally.
	to = collide_and_slide(from, to, radius, collider_capsule_height, downstep, upstep, true, 5)
	
	-- Move down.
	local ground = to + (vec3.down * collider_vertical_offset)
	from = ground + (vec3.up * (radius+upstep))
	to = from + (vec3.down * (upstep+downstep))
	to = collide_and_slide(from, to, radius*0.99, collider_capsule_height, downstep, upstep, false, 3)
	
	return to + (vec3.down * radius)
end

function collide_and_slide(from, to, radius, height, downstep, upstep, is_horizontal, num_iterations)

	if num_iterations <= 0 then return from end

	local speed = vec3.distance(from, to)
	if speed < 0.00001 then
		return from
	end
	
	local movedir = (to-from):normalized()
	
	-- Use a sphere cast to see if we'll hit anything.
	local result = nil
	if is_horizontal then
		result = physics.capsulecast(from, to, radius*0.99, height, {"scenery"})
	else
		result = physics.spherecast(from, to, radius*0.99, {"scenery"})
	end
	
	if result.hit == false then
		return to
	end
	
	-- Calculate where we end up based on the collision result.
	local new_position = result.position + (result.normal * radius)
	if is_horizontal then
		new_position.z = from.z
		result.normal.z = 0
		result.normal:normalize()
	else
		nrmdetect = physics.cylindercast(new_position, new_position+(vec3.down*(radius+height)), radius*0.707, 0.1, {"scenery"})
		if nrmdetect.hit then
			result.normal = nrmdetect.normal
		end
		if result.normal.z > flat_ground_threshold then
			new_position.xy = from.xy
			return new_position
		end
	end
	
	local angle = vec3.angle(result.normal, movedir)
	local hypotenuse = vec3.distance(new_position, to)
	local goal_to_wall_dist = hypotenuse * math.sin((math.pi*0.5)-angle)
	local new_destination = to - (result.normal * goal_to_wall_dist)
	
	-- Slow down our vertical movement when colliding with a slope.
	if is_horizontal == false then
		local newspeed = vec3.distance(new_position, new_destination)
		if newspeed < 0.00001 then
			return new_position
		end
		local newdir = (new_destination - new_position):normalized()
		local remaining_speed = speed - vec3.distance(from, new_position)
		
		newspeed = math.lerp(newspeed, 0, math.linstep(result.normal.z, too_steep_threshold, flat_ground_threshold))
		
		new_destination = new_position + (newdir * newspeed)
	end
	
	return collide_and_slide(new_position, new_destination, radius, height, downstep, upstep, is_horizontal, num_iterations-1)
end