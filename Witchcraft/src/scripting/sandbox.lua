R"==(--sandbox.lua

--- Sets up tables and functions to allow scripts run by the Witchcraft engine
--- to run in their own protected sandbox environments.
-- Written by Haydn V. "Candlemaster" Harach on 3/21/2016
-- Last modified on 3/29/2018

-- The 'readonly' function makes it simple to define read-only tables.
function readonly(tbl)
	return setmetatable({}, {
	__index = tbl,
	__newindex = function(tbl, key, val)
		error("Attempting to write to read-only table.") return end,
	__metatable = false
	})
end

-- The 'showtable' function allows us to print the entire table's contents - including the contents of any tables inside!
showtable = function(t, depth, set)

	if not depth then
		depth = ""
	end
	
	if not set then
		set = {}
	end
	
	set[t] = true
	
	for k,v in pairs(t) do
		if type(v) == 'table' then
			print(depth, k, ":")
			if set[t] then
				print(depth, k, ": (already encountered)")
			else
				showtable(v, depth.."  ", set)
			end
		else
			print(depth, k, ": ", v)
		end
	end
	
end

function spring(current, velocity, target, tightness, delta_time)
	local current_to_target = target - current
	local spring_force = current_to_target * tightness
	local damping_force = -velocity * 2 * math.sqrt(tightness)
	local force = spring_force + damping_force
	velocity = velocity + (force * delta_time)
	local displacement = velocity * delta_time
	current = current + displacement
	return current, velocity
end

function angle_difference(lhs, rhs)
	local diff = (((rhs - lhs) + 180) % 360) - 180
	if diff < -180 then
		return diff + 360
	else
		return diff
	end
end

function radians_difference(lhs, rhs)
	local diff = (((rhs - lhs) + math.pi) % (math.pi * 2) - math.pi)
	if diff < -math.pi then
		return diff + (math.pi * 2)
	else
		return diff
	end
end

function spring_angle(current, velocity, target, tightness, delta_time)
	local current_to_target = angle_difference(current, target)
	local spring_force = current_to_target * tightness
	local damping_force = -velocity * 2 * math.sqrt(tightness)
	local force = spring_force + damping_force
	velocity = velocity + (force * delta_time)
	local displacement = velocity * delta_time
	current = current + displacement
	return current, velocity
end

function spring_radians(current, velocity, target, tightness, delta_time)
	local current_to_target = radians_difference(current, target)
	local spring_force = current_to_target * tightness
	local damping_force = -velocity * 2 * math.sqrt(tightness)
	local force = spring_force + damping_force
	velocity = velocity + (force * delta_time)
	local displacement = velocity * delta_time
	current = current + displacement
	return current, velocity
end

function sign(val)
	if val < 0 then return -1 else return 1 end
end

function clamp(val, minimum, maximum)
	if maximum < minimum then minimum, maximum = maximum, minimum end
	return math.min(maximum, math.max(minimum, val))
end

function linstep(value, minimum, maximum)
	return clamp((value-minimum) / (maximum - minimum), 0, 1)
end

function lerp(from, to, alpha)
	return (from * (1 - alpha)) + (to * alpha)
end

SANDBOX_SCRIPT_ACCESS = {}

-- The 'SANDBOX' table contains everything from _G that we want user-run scripts to have access to.
-- It also contains proxy tables that give user scripts read-only access to other user scripts.
SANDBOX = {

	assert = _G.assert,
--	dofile = _G.dofile, -- Redefined by the engine
	error = _G.error,
	ipairs = _G.ipairs,
	next = _G.next,
	pairs = _G.pairs,
	pcall = _G.pcall,
--	print = _G.print, -- Redefined
--	require = _G.require, -- Redefined
	select = _G.select,
	tonumber = _G.tonumber,
	tostring = _G.tostring,
	type = _G.type,
	_VERSION = _G._VERSION,
	xpcall = _G.xpcall,
	
	readonly = _G.readonly,
	showtable = _G.showtable,
--	loadXML = _G.loadXML, -- Defined by the engine
	
	
	-- For tables inside _G that we want access to, we create proxy tables which provide
	-- read-only access to only the functions and variables that we want.
	coroutine = readonly({
		create = _G.coroutine.create,
		resume = _G.coroutine.resume,
		running = _G.coroutine.running,
		status = _G.coroutine.status,
		wrap = _G.coroutine.wrap,
		yield = _G.coroutine.yield
	}),
	
	math = readonly({
		abs = _G.math.abs,
		acos = _G.math.acos,
		asin = _G.math.asin,
		atan = _G.math.atan,
		atan2 = _G.math.atan2,
		ceil = _G.math.ceil,
		cos = _G.math.cos,
		cosh = _G.math.cosh,
		deg = _G.math.deg,
		exp = _G.math.exp,
		floor = _G.math.floor,
		fmod = _G.math.fmod,
		frexp = _G.math.frexp,
		huge = _G.math.huge,
		ldexp = _G.math.ldexp,
		log = _G.math.log,
		log10 = _G.math.log10,
		max = _G.math.max,
		min = _G.math.min,
		modf = _G.math.modf,
		pi = _G.math.pi,
		pow = _G.math.pow,
		rad = _G.math.rad,
		random = _G.math.random,
		sin = _G.math.sin,
		sinh = _G.math.sinh,
		sqrt = _G.math.sqrt,
		tan = _G.math.tan,
		tanh = _G.math.tanh,
		
		-- The following isn't part of vanilla Lua, but useful enough to be here.
		tau = _G.math.pi * 2,
		deg2rad = _G.math.pi / 180,
		rad2deg = 180 / _G.math.pi,
		
		spring = _G.spring,
		spring_angle = _G.spring_angle,
		spring_radians = _G.spring_radians,
		angle_difference = _G.angle_difference,
		radians_difference = _G.radians_difference,
		sign = _G.sign,
		clamp = _G.clamp,
		linstep = _G.linstep,
		lerp = _G.lerp,
	}),
	
	os = readonly({
		clock = _G.os.clock,
		date = _G.os.date,
		difftime = _G.os.difftime,
		time = _G.os.time
	}),
	
	string = readonly({
		byte = _G.string.byte,
		char = _G.string.char,
		find = _G.string.find,
		format = _G.string.format,
		gmatch = _G.string.gmatch,
		gsub = _G.string.gsub,
		len = _G.string.len,
		lower = _G.string.lower,
		match = _G.string.match,
		rep = _G.string.rep,
		reverse = _G.string.reverse,
		sub = _G.string.sub,
		upper = _G.string.upper
	}),
	
	table = readonly({
		insert = _G.table.insert,
		maxn = _G.table.maxn,
		remove = _G.table.remove,
		sort = _G.table.sort,
		unpack = _G.table.unpack,
	}),

--	vec2 = readonly(_G.vec2),
--	vec3 = readonly(_G.vec3),
--	vec4 = readonly(_G.vec4),
--	quat = readonly(_G.quat),
	
	script = readonly(SANDBOX_SCRIPT_ACCESS),
}

-- The 'SCRIPT_ENV' table holds the tables which will be the environment for user-run scripts.
-- Each script gets it's own environment, shared only by scripts with the same name.
SCRIPT_ENV = {}

-- The 'SCRIPT_ENV_PROTECTED' table holds things that are specific to a particular script's env,
-- but are read-only even within that script's own environment.  Things like class internals, etc.
SCRIPT_ENV_PROTECTED = {}

-- To clear or re-initialize the script environment (for testing), set SANDBOX.script, SCRIPT_ENV, and SCRIPT_ENV_PROTECTED all to empty tables.

-- Every script environment has the same metatable, so we save it in _G to re-use.
-- While the script runs in it's own environment, 'SCRIPT_ENV.scriptName',
-- it has access to 'SANDBOX' through the __index metamethod.
-- __newindex prevents the script from creating a value with the same key as something in the sandbox,
-- as doing so would make that feature inaccessible.
SCRIPT_ENV_MT = {
	__index = SANDBOX,
	__newindex = function(tbl, key, val)
		if SANDBOX[key] ~= nil then
			error("Attempting to overwrite protected key '"..tostring(key).."'.")
			return end
		rawset(tbl, key, val)
	end,
	__metatable = false
}

-- The 'CONSOLE' table is the environment used by lua strings run in the console prompt.
-- 'CONSOLE_PROTECTED' will contain any functions usable from the console that aren't usable by normal scripts.
CONSOLE_PROTECTED = setmetatable({},
{
	__index = SANDBOX,
	__metatable = false
})

CONSOLE_MT = {
	__index = CONSOLE_PROTECTED,
	__newindex = function(tbl, key, val)
	if CONSOLE_PROTECTED[key] ~= nil then
		error ("Attempting to overwrite protected key '"..tostring(key).."'.")
		return
		end
	rawset(tbl, key, val)
	end,
	__metatable = false
}

CONSOLE = setmetatable({}, CONSOLE_MT)

CONSOLE_PROTECTED.reset_console = function()
	CONSOLE = setmetatable({}, CONSOLE_MT)
end

-- setup_script_env() is run by the engine before a user script is run, setting up the environment for it.
function setup_script_env(scriptname)

	-- If we've set up a table for 'scriptname' before, we bail out now.
	if SCRIPT_ENV[scriptname] then return end

	-- Don't let the user give their script the same name as something already in 'SANDBOX'.
	if SANDBOX[scriptname] then
		error("Cannot create environment for '"..scriptname.."' as it is a reserved key.")
		return
	end
	
	
	-- Create the table for 'scriptname', and a protected table as well.
	-- The protected table is used whenever the engine sets up methods for the script,
	-- ie. making it behave like a class.
	-- SCRIPT_ENV[i] is where the script runs.  It's index points to SCRIPT_ENV_PROTECTED[i].
	-- SCRIPT_ENV_PROTECTED[i] is where engine-defined per-script methods go.  It's index points to SANDBOX.
	SCRIPT_ENV_PROTECTED[scriptname] = setmetatable({_SCRIPTNAME = scriptname}, SCRIPT_ENV_MT)
	SCRIPT_ENV[scriptname] = setmetatable({}, {
		__index = SCRIPT_ENV_PROTECTED[scriptname],
		__newindex = function(tbl, key, val)
			if SCRIPT_ENV_PROTECTED[scriptname][key] then
				error("Attempting to overwrite protected key '"..tostring(key).."'.")
				return end
			rawset(tbl, key, val)
		end,
		__metatable = false
	})
	
	-- Add a proxy table named 'scriptname' to 'SANDBOX.script',
	-- allowing other scripts read-only access to the contents of 'SCRIPT_ENV[scriptname]'.
	-- Note that the contents of any tables inside 'SCRIPT_ENV[scriptname]' will be writeable by default.
	SANDBOX_SCRIPT_ACCESS[scriptname] = readonly(SCRIPT_ENV[scriptname])
end

)=="