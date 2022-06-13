-- freecam.lua
--camera = entity.create()
--camera:setname("Free Camera")
--camera:add_transform()
--camera:setpos(0, 0, 1)
--camera:setrot(quat.look(vec3.forward))
--renderer.set_camera_entity(camera)


local tilefloor = entity.create()
tilefloor:setname("Floor")
tilefloor:add_transform()
tilefloor:add_renderable()
tilefloor:load_model('test_level.wcm.xml')
tilefloor:make_collider_from_model()
tilefloor:add_rigidbody(0, false, false, "scene mesh", {"scenery mask"})
--[[
local box = entity.create()
box:add_transform()
box:setpos(1.5, 1.5, 0)
box:add_renderable()
box:load_model('xbot.wcm.xml')
events.logical_update:listen(box, "freecam", function(self)
	self:rotate(quat.euler(0, 0, math.deg2rad*60*time.delta_time))
end)


box = entity.create()
box:add_transform()
box:setpos(1.5, -1.5, 0)
box:add_renderable()
box:load_model('unitychan.wcm.xml')
events.logical_update:listen(box, "freecam", function(self)
	self:rotate(quat.euler(0, 0, math.deg2rad*60*time.delta_time))
end)

box = entity.create()
box:add_transform()
box:setpos(-1.5, 1.5, 0)
box:add_renderable()
box:load_model('unitychan2.wcm.xml')
box:add_animator()
events.logical_update:listen(box, "freecam", function(self)
	--self:rotate(quat.euler(0, math.deg2rad*60*time.delta_time, 0))
end)



box = entity.create()
box:add_transform()
box:setpos(-1.5, -1.5, 0)
box:add_renderable()
box:load_model('unitychan2.wcm.xml')
box:add_animator()
box:import_animations('unitychan2_RunF.wcm.xml')
box:import_animations('unitychan2_Idle.wcm.xml')
box:import_animations("unitychan2_Walk.wcm.xml")
box:play_animation(0, 'Idle', true, 1)
events.logical_update:listen(box, "freecam", function(self)
	--self:rotate(quat.euler(0, math.deg2rad*60*time.delta_time, 0))
end)

local unitychan = box

box = entity.create()
box:add_transform()
box:setscale(0.02, 0.2, 0.02)
box:add_renderable()
box:load_model('testcube.wcm.xml')
events.logical_update:listen(box, 'freecam', function(self)
	local animpos, animrot = unitychan:get_animated_transform('Character1_LeftHand')
	self:setpos(animpos)
	self:setrot(animrot)
end)
--]]


renderer.load_skybox('textures/envmap_stormydays/stormydays')


local player = entity.create()
player:add_script('player_controller')


--local floorbox = entity.create()
--floorbox:add_transform()
--floorbox:setpos(0, 0, -1)
--floorbox:add_collider("box", 50, 50, 1)
--floorbox:add_rigidbody(0, false, "scene object", {"scenery mask"})

--local fallingbox = entity.create()
--fallingbox:add_transform()
--fallingbox:setpos(0, 10, 10)
--fallingbox:add_renderable()
--fallingbox:load_model('testcube.wcm.xml')
--fallingbox:add_collider("box", 1, 1, 1)
--fallingbox:add_rigidbody(1, false, "clutter", {"clutter mask"})


--local state = {}

--state[camera:index()] = {
--	yaw = 0,
--	pitch = 0
--}

--params =
--{
--	speed = 10,
--	yawspeed = -3,
--	pitch = {
--		speed = 3,
--		minimum = -89 * math.deg2rad,
--		maximum = 89 * math.deg2rad
--	}
--}

--function onLogicalUpdate(self)

--	local mystate = state[self:index()]
--	local lthumb = input.controls.get('movement')
--	local rthumb = input.controls.get('camera')
	
--	mystate.yaw = mystate.yaw + (rthumb.x * params.yawspeed * time.delta_time)
--	mystate.pitch = mystate.pitch + (rthumb.y * params.pitch.speed * time.delta_time)
--	mystate.pitch = math.clamp(mystate.pitch, params.pitch.minimum, params.pitch.maximum)
	
--	if input.gamepad.is_down("a") then
--		print("Pressed A");
--	end

--	local fall = input.gamepad.triggers().y
--	if input.key.is_down('f') then
--		fall = 1
--	end
--	local rise = input.gamepad.triggers().x
--	if input.key.is_down('r') then
--		rise = 1
--	end
--	self:move(0, 0, (rise - fall) * params.speed * time.delta_time)
	
--	if lthumb:magnitude_squared() ~= 0 then
--		local forward = vec3.new(lthumb, 0)
--		forward:normalize()
--		forward = quat.euler(0, 0, mystate.yaw) * forward
--		self:move(forward * lthumb:magnitude() * params.speed * time.delta_time)
--	end
--	
--	self:setrot(quat.euler(mystate.pitch, 0, mystate.yaw))
--end

--events.logical_update:listen(camera, "freecam", onLogicalUpdate)