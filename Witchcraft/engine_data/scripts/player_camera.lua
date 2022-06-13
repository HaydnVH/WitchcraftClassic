-- player_camera.lua

local state = {}

params = 
{
	distance = 
	{
		min = 1,
		max = 3,
		start = 1.45,
	},
	
	menu_distance =
	{
		min = 1.5,
		max = 2,
		start = 2,
	},
	
	pitch = 
	{
		min = math.deg2rad*-70,
		max = math.deg2rad*50,
		start = math.deg2rad*-10,
		speed = math.deg2rad*270,
		look_up = 0.75,
	},
	
	yaw = 
	{
		speed = -math.deg2rad*270
	},
	
	fov = 
	{
		min = 50,
		max = 110,
		start = 80,
	},
	
	springs = 
	{
		yaw = 350,
		pitch = 350,
		state_blend = 150
	},
	
--	last_movement_delay = 1,
--	last_cam_input_delay = 1,
	
	pivot = true,
	pivot_mult = 1,
	
	focus_offset = vec3.new(0, 0, 1.15),
	
	firstperson =
	{
		offset = vec3.new(0, 0, 1.7),
		min_pitch = math.deg2rad*-89,
		max_pitch = math.deg2rad*89,
		fov = 90,
	},
}

function on_init(self)

	renderer.set_camera_entity(self)

	state[self:index()] = {}
	local mystate = state[self:index()]
	
	mystate.player = entity.find_with_name('Player')
	
	mystate.state_func = ThirdPerson
	mystate.old_state_table = nil
	mystate.current_state_table = nil
	mystate.state_blend_ideal = 1
	mystate.state_blend_true = 1
	mystate.state_blend_velocity = 0
	
	mystate.pitch_ideal = params.pitch.start
	mystate.pitch_true = params.pitch.start
	mystate.pitch_velocity = 0
	
	mystate.firstperson = false
	
	-- Set the initial yaw according to the player's direction
	local look_dir = mystate.player:forward()
	look_dir.z = 0
	look_dir:normalize()
	mystate.yaw_ideal = vec3.signed_angle(vec3.forward, look_dir)
	mystate.yaw_true = mystate.yaw_ideal
	mystate.yaw_velocity = 0
	
	-- Set the camera's rotation and position
	local pivot_position = mystate.player:getpos() + params.focus_offset
	mystate.ideal_position = pivot_position + ((quat.euler(mystate.pitch_true, 0, mystate.yaw_true) * vec3.forward) * -params.distance.start)
	self:setpos(mystate.ideal_position)
	self:setrot(quat.euler(mystate.pitch_true, 0, mystate.yaw_true))
	
--	mystate.last_movement_time = 0
--	mystate.last_yaw_input_time = 0
--	mystate.last_pitch_input_time = 0
--	mystate.is_yaw_player_controlled = false
--	mystate.is_pitch_player_controlled = false
end

function OnRemoveFromEntity(self)
	state[self:index()] = nil
end

function ChangeState(self, newfunc)
	local mystate = state[self:index()]
	
	mystate.old_state_table = mystate.current_state_table
	mystate.state_func = newfunc
	mystate.state_blend_true = 0
end

function OnForcedUpdate(self)
	local mystate = state[self:index()]
	
	mystate.current_state_table = mystate.state_func(self)
	
	local fov
	local dist
	local offset
	
	if mystate.old_state_table ~= nil then
		mystate.state_blend_true, mystate.state_blend_velocity = math.spring(mystate.state_blend_true, mystate.state_blend_velocity, mystate.state_blend_ideal, params.springs.state_blend, time.delta_time)
		
		fov = math.lerp(mystate.old_state_table.fov, mystate.current_state_table.fov, mystate.state_blend_true)
		dist = math.lerp(mystate.old_state_table.distance, mystate.current_state_table.distance, mystate.state_blend_true)
		offset = math.lerp(mystate.old_state_table.pivot_offset, mystate.current_state_table.pivot_offset, mystate.state_blend_true)
	else
		fov = mystate.current_state_table.fov
		dist = mystate.current_state_table.distance
		offset = mystate.current_state_table.pivot_offset
	end
	
	mystate.yaw_true = ((mystate.yaw_true + math.tau) % (math.tau * 2)) - math.tau
	mystate.yaw_ideal = ((mystate.yaw_ideal + math.tau) % (math.tau * 2)) - math.tau
	
	-- Update the camera's transform
	local pivot_position = mystate.player:getpos() + offset
	mystate.ideal_position = pivot_position + ((quat.euler(mystate.pitch_true, 0, mystate.yaw_true) * vec3.forward) * -dist)
	self:setpos(mystate.ideal_position)
	self:setrot(quat.euler(mystate.pitch_true, 0, mystate.yaw_true))
	
	if input.controls.on_down('first person') then
		mystate.firstperson = not mystate.firstperson
		if mystate.firstperson then
			ChangeState(self, FirstPerson)
		else
			ChangeState(self, ThirdPerson)
		end
	end
	
end

function ThirdPerson(self)
	local mystate = state[self:index()]
	local rthumb = input.controls.get('camera')
	local lthumb = input.controls.get('movement')
	local btn_defend = input.controls.get('defend')
	
	local result_table = {}
	result_table.pivot_offset = params.focus_offset
	local pivot_position = mystate.player:getpos() + result_table.pivot_offset
	
	-- Adjust yaw according to player input
	mystate.yaw_ideal = mystate.yaw_ideal + (rthumb.x * params.yaw.speed * math.lerp(1, 0.666, lthumb:magnitude()) * time.delta_time)
	
	-- Clamp pitch, in case it was set beyond these limits in first-person mode
	mystate.pitch_ideal = math.clamp(mystate.pitch_ideal, params.pitch.min, params.pitch.max)
	
	-- Adjust pitch according to player input
--	if mystate.is_pitch_player_controlled then
		local pitch_speed_mod = 1
		if rthumb.y > 0 then
			if params.pitch.speed < 0 and mystate.pitch_ideal < 0 then
				-- We're tilting the camera towards the ground
				local pitch_normalized = (mystate.pitch_ideal - params.pitch.start) / (params.pitch.min - params.pitch.start)
				pitch_speed_mod = math.log(2 - pitch_normalized)
			elseif params.pitch.speed >= 0 and mystate.pitch_ideal >= 0 then
				-- We're tilting the camera towards the sky
				local pitch_normalized = (mystate.pitch_ideal - params.pitch.start) / (params.pitch.max - params.pitch.start)
				pitch_speed_mod = math.log(2 - pitch_normalized)
			end
		elseif rthumb.y < 0 then
			if params.pitch.speed >= 0 and mystate.pitch_ideal < 0 then
				-- We're tilting the camera towards the ground
				local pitch_normalized = (mystate.pitch_ideal - params.pitch.start) / (params.pitch.min - params.pitch.start)
				pitch_speed_mod = math.log(2 - pitch_normalized)
			elseif params.pitch.speed < 0 and mystate.pitch_ideal >= 0 then
				-- We're tilting the camera towards the sky
				local pitch_normalized = (mystate.pitch_ideal - params.pitch.start) / (params.pitch.max - params.pitch.start)
				pitch_speed_mod = math.log(2 - pitch_normalized)
			end
		end
		
		mystate.pitch_ideal = mystate.pitch_ideal + (rthumb.y * params.pitch.speed * pitch_speed_mod * time.delta_time)
--	end
	
	-- Clamp pitch
	mystate.pitch_ideal = math.clamp(mystate.pitch_ideal, params.pitch.min, params.pitch.max)
	
	-- Adjust the yaw, taking the player's current position into account.
	-- This allows the player to run a circle around the camera by holding left or right on the thumbstick.
	if (params.pivot == true) and (btn_defend == false) then
		-- look_dir_a is the vector from where the camera WAS last frame, to where the player IS now
		local look_dir_a = pivot_position - mystate.ideal_position
		look_dir_a.z = 0
		look_dir_a:normalize()
		
		-- look_dir_b is the vector from where the camera WAS last frame, to where the player WAS last frame
		local look_dir_b = self:forward()
		look_dir_b.z = 0
		look_dir_b:normalize()
		
		local looking_around_modifier = 1 - rthumb:magnitude()
		
		-- the angle between these two vectors should be what we have to correct our yaw by
		local correction = vec3.signed_angle(look_dir_b, look_dir_a) * params.pivot_mult * looking_around_modifier * math.abs(lthumb.x)
		mystate.yaw_ideal = mystate.yaw_ideal + correction
	end
	
	-- When the player presses 'defend' (left trigger), swing the camera to face the same direction as the player
	if input.controls.on_down('defend') then
		-- Set the initial yaw according to the player's direction
		local look_dir = mystate.player:forward()
		look_dir.z = 0
		look_dir:normalize()
		mystate.yaw_ideal = vec3.signed_angle(vec3.forward, look_dir)
		
		mystate.pitch_ideal = params.pitch.start
	end
	
	-- Detect walls here, I guess?
	
	mystate.yaw_true, mystate.yaw_velocity = math.spring_radians(mystate.yaw_true, mystate.yaw_velocity, mystate.yaw_ideal, params.springs.yaw, time.delta_time)
	mystate.pitch_true, mystate.pitch_velocity = math.spring(mystate.pitch_true, mystate.pitch_velocity, mystate.pitch_ideal, params.springs.pitch, time.delta_time)
	
	-- Adjust the camera's field of view according to the true pitch
	local real_pitch = mystate.pitch_true
	if real_pitch > (180 * math.deg2rad) then
		real_pitch = real_pitch - (360 * math.deg2rad)
	end
	if real_pitch > params.pitch.start then
		local pitch_normalized = (real_pitch - params.pitch.start) / (params.pitch.max - params.pitch.start)
		result_table.fov = params.fov.start + (pitch_normalized * pitch_normalized * (params.fov.max - params.fov.start))
		result_table.pivot_offset = result_table.pivot_offset + (vec3.up * pitch_normalized * pitch_normalized * params.pitch.look_up)
	else
		result_table.fov = params.fov.start
	end
	
	result_table.distance = params.distance.start
	
	return result_table
end

function FirstPerson(self)
	local mystate = state[self:index()]
	local rthumb = input.controls.get('camera')
	
	local result_table = {}
	
	-- Adjust yaw according to player input
	mystate.yaw_ideal = mystate.yaw_ideal + (rthumb.x * params.yaw.speed * time.delta_time)
	
	-- Adjust pitch according to player input
	mystate.pitch_ideal = mystate.pitch_ideal + (rthumb.y * params.pitch.speed * time.delta_time)
	mystate.pitch_ideal = math.clamp(mystate.pitch_ideal, params.firstperson.min_pitch, params.firstperson.max_pitch)
	
	
	mystate.yaw_true, mystate.yaw_velocity = math.spring_radians(mystate.yaw_true, mystate.yaw_velocity, mystate.yaw_ideal, params.springs.yaw, time.delta_time)
	mystate.pitch_true, mystate.pitch_velocity = math.spring(mystate.pitch_true, mystate.pitch_velocity, mystate.pitch_ideal, params.springs.pitch, time.delta_time)
	
--	mystate.yaw_true = mystate.yaw_ideal
--	mystate.yaw_velocity = 0
--	mystate.pitch_true = mystate.pitch_ideal
--	mystate.pitch_velocity = 0

	result_table.fov = params.firstperson.fov
	result_table.distance = 0
	result_table.pivot_offset = params.firstperson.offset
	
	return result_table
end

function show_ideal_position(self)
	print('Ideal camera position = ', state[self:index()].ideal_position)
end

function show_table(self)
	print('-----')
	showtable(state[self:index()])
	print('\n')
end

function GetTrueForward(self)
	local mystate = state[self:index()]
	
	if mystate == nil then
		return quat.identity
	else
		return quat.euler(0, 0, mystate.yaw_ideal) * vec3.forward
	end
end

function GetTrueYaw(self)
	return state[self:index()].yaw_true
end

function GetIdealYaw(self)
	return state[self:index()].yaw_ideal
end

function force_first_person(self, arg)
	local mystate = state[self:index()]
	if arg == nil then
		mystate.firstperson = not mystate.firstperson
	else
		mystate.firstperson = arg
	end
end
