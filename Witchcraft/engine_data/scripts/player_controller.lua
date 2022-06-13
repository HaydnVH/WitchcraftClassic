-- player_controller.lua

dofile("movement_collision_test")

local state = {}

params = 
{
	walk_speed = 1,
	run_speed = 4,
	sprint_speed = 12,
	acceleration = 20,
	decceleration = 60,
	friction = 20,
	turn_speed = math.deg2rad*30,
	
	collider_radius = 0.333,
	character_height = 1.55,
	downstep = 0.2,
	upstep = 0.1,
}

function on_init(self)

	self:play_animation(0, "Idle", true, 0.5)

	state[self:index()] = {}
	local mystate = state[self:index()]
	
	mystate.camera = nil
	
	mystate.h_velocity = vec3.new(0,0,0)
	mystate.animrunning = false
	
	events.logical_update:listen(self, "player_controller", OnLogicalUpdate)
end

function on_cleanup(self)
--	entity.destroy(state[self:index()].camera)
	state[self:index()] = nil
	events.OnLogicalUpdate:unlisten(self, "player_controller")
end

function OnLogicalUpdate(self)
	local mystate = state[self:index()]
	
	if mystate.camera == nil then
		mystate.camera = entity.find_with_name('Camera')
	end
	
	local stick_direction = vec3.new(input.controls.get("movement").x, input.controls.get("movement").y, 0)
	
	local max_speed = stick_direction:magnitude()
	if max_speed > 0 then
		max_speed = math.lerp(params.walk_speed, params.run_speed, max_speed)
	end
	
	local camera_direction = script.player_camera.GetTrueForward(mystate.camera)
	camera_direction.z = 0
	camera_direction:normalize()
	
	local move_direction = quat.look(camera_direction) * stick_direction
	
	local old_velocity = mystate.h_velocity:normalized()
	
	if mystate.h_velocity:magnitude() == 0 then
		-- The player is currently at standstill.
		-- We can set the direction directly and increase velocity by acceleration;
		-- If there's no input then this will simply equal zero.
		mystate.h_velocity = move_direction * params.acceleration * time.delta_time
		
		
	elseif move_direction:magnitude() == 0 then
		-- The player is currently moving, but we have no input.
		-- We can simply decrease the velocity, in this case.
		if mystate.h_velocity:magnitude() < params.friction * time.delta_time then
			mystate.h_velocity = vec3.new(0,0,0)
		else
			mystate.h_velocity = mystate.h_velocity - (mystate.h_velocity:normalized() * params.friction * time.delta_time)
		end
		
	else
		-- In this case, we have both existing velocity and player input.
		
		-- Scale should be a value equalling 1 if we're going straight, and 0 if we're going directly backwards.
		local turn_scale = (vec3.dot(mystate.h_velocity:normalized(), move_direction:normalized()) + 1) * 0.5
		local old_speed = mystate.h_velocity:magnitude()
		local new_velocity = mystate.h_velocity + (move_direction * math.lerp(params.decceleration, params.acceleration, turn_scale) * time.delta_time)
		
		-- If our old speed is less than our max speed, 
		if old_speed < max_speed then
			-- update and then cap our velocity.
			if new_velocity:magnitude() > max_speed then
				mystate.h_velocity = new_velocity:normalized() * max_speed
			else
				mystate.h_velocity = new_velocity
			end
		
		-- If our old speed is greater than our max speed,
		elseif old_speed > max_speed then
			-- We apply friction.
			if old_speed - params.friction * time.delta_time < max_speed then
				mystate.h_velocity = new_velocity:normalized() * max_speed
			else
				mystate.h_velocity = new_velocity:normalized() * (old_speed - (params.friction * time.delta_time))
			end
		
		-- If our old speed is exactly equal to our max speed (unlikely)
		else
			mystate.h_velocity = new_velocity:normalized() * max_speed
		end
	end
	

	if mystate.h_velocity:magnitude() > 0 then
		if mystate.animrunning == false then
			self:play_animation(0, "Walk", true, 1.6, 0.12)
			self:play_animation(1, "RunF", true, 1, 0.12)
			mystate.animrunning = true
		end
		local remapped_speed = math.linstep(mystate.h_velocity:magnitude(), params.walk_speed, params.run_speed)
		self:set_anim_layer_weight(0, 1 - remapped_speed)
		self:set_anim_layer_weight(1, remapped_speed)
		self:set_animation_speed(mystate.h_velocity:magnitude() / params.run_speed)
	else
		if mystate.animrunning == true then
			self:play_animation(0, "Idle", true, 1, 0.12)
			self:play_animation(1, "nil", true, 1, 0.12)
			self:set_anim_layer_weight(0, 1)
			self:set_anim_layer_weight(1, 0)
			mystate.animrunning = false
		end
		self:set_animation_speed(1);
	end

	
	-- If we're moving, then we need to update our rotation.
	if input.controls.get("defend") == true and input.controls.on_down("defend") == false then
		local angle = script.test_player_camera.GetIdealYaw(mystate.camera)
		self:setrot(quat.euler(0, 0, angle + math.pi))
	elseif mystate.h_velocity:magnitude() > 0 then
		local angle = vec3.signed_angle(self:getrot() * vec3.forward, mystate.h_velocity:normalized())
		self:rotate(quat.axis_angle(vec3.up, angle))
	end
	
	-- Bump into the world.
	local from = self:getpos()
	local to = from + (mystate.h_velocity * time.delta_time)
	to = script.movement_collision_test.handle_movement_collisions(from, to, params.collider_radius, params.character_height, params.downstep, params.upstep)
	
	-- Finally, we update our position.
	self:setpos(to)
	
	
	script.player_camera.OnForcedUpdate(mystate.camera)
	
	self:draw_skeleton_debug()
	
	-- Skeleton debugging
	--[[
	local pelvispos = self:get_animated_transform('Pelvis')
	local torsopos = self:get_animated_transform('Torso')
	local neckpos, dummyrot = self:get_animated_transform('Neck')
	local headpos, dummyrot = self:get_animated_transform('Head')
	local lbiceppos, dummyrot = self:get_animated_transform('Left Bicep')
	local lforearmpos, dummyrot = self:get_animated_transform('Left Forearm')
	local lhandpos, dummyrot = self:get_animated_transform('Left Hand')
	local rbiceppos, dummyrot = self:get_animated_transform('Right Bicep')
	local rforearmpos, dummyrot = self:get_animated_transform('Right Forearm')
	local rhandpos, dummyrot = self:get_animated_transform('Right Hand')
	local lthighpos, dummyrot = self:get_animated_transform('Left Thigh')
	local lcalfpos, dummyrot = self:get_animated_transform('Left Calf')
	local lfootpos, dummyrot = self:get_animated_transform('Left Foot')
	local rthighpos, dummyrot = self:get_animated_transform('Right Thigh')
	local rcalfpos, dummyrot = self:get_animated_transform('Right Calf')
	local rfootpos, dummyrot = self:get_animated_transform('Right Foot')
	
	debug.drawline(pelvispos, torsopos, {0.3, 0.8, 0.8})
	debug.drawline(torsopos, neckpos, {0.3, 0.8, 0.8})
	debug.drawline(neckpos, headpos, {0.3, 0.8, 0.8})
	debug.drawline(neckpos, lbiceppos, {0.3, 0.8, 0.8})
	debug.drawline(lbiceppos, lforearmpos, {0.3, 0.8, 0.8})
	debug.drawline(lforearmpos, lhandpos, {0.3, 0.8, 0.8})
	debug.drawline(neckpos, rbiceppos, {0.3, 0.8, 0.8})
	debug.drawline(rbiceppos, rforearmpos, {0.3, 0.8, 0.8})
	debug.drawline(rforearmpos, rhandpos, {0.3, 0.8, 0.8})
	debug.drawline(pelvispos, lthighpos, {0.3, 0.8, 0.8})
	debug.drawline(lthighpos, lcalfpos, {0.3, 0.8, 0.8})
	debug.drawline(lcalfpos, lfootpos, {0.3, 0.8, 0.8})
	debug.drawline(pelvispos, rthighpos, {0.3, 0.8, 0.8})
	debug.drawline(rthighpos, rcalfpos, {0.3, 0.8, 0.8})
	debug.drawline(rcalfpos, rfootpos, {0.3, 0.8, 0.8})
	--]]
	
	
--	self:set_bone_additive_rotation('Character1_Head', quat.euler(math.sin(time.now), 0, 0))

--[[
	to = from + (self:forward() * 10)
	local rayresult = physics.spherecast(from, to, 0.333 * 0.9, {"scenery", "clutter"})
	if rayresult.hit == true then
		local spherecenter = rayresult.sweep_pos
		debug.drawsphere(spherecenter, 0.333, {1, 0, 1})
		debug.drawline(from, spherecenter, {1, 0, 1})
		debug.drawline(rayresult.position, rayresult.position + (rayresult.normal * 1), {1, 1, 0})
	else
		debug.drawline(from, to, {0, 1, 1})
	end
--]]
end