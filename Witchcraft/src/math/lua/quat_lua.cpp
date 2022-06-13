#include "vmath_lua.h"

#include "../vec2.h"
#include "../vec3.h"
#include "../vec4.h"
#include "../quat.h"

#include "sys/printlog.h"
#include "scripting/luasystem.h"

namespace vmath {

int lua_quat_new(lua_State* L)
{
	// If the first argument on the stack is a table, then we're creating a quat using "var = quat()".
	// If it isn't, then we're creating it using "var = quat.new()".
	if (lua_istable(L, 1))
	{
		// If the first arg is the table, let's just remove it.
		lua_remove(L, 1);
	}

	// Get the number of arguments on the stack.  Should be 0, 1, 2, or 3.
	int argc = lua_gettop(L);

	// Allocate space for the new quat within lua.
	quat* self = (quat*)lua_newuserdata(L, sizeof(quat));
	// Associate the "quat" metatable with the value we just created.
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	switch (argc)
	{
	case 0: // 0 arguments, therefore default constructor.
		*self = QUAT_IDENTITY;
		break;
	case 1: // 1 argument, should be a quat.
	{
		quat* rhs = (quat*)luaL_checkudata(L, 1, "quat");
		*self = *rhs;
	}
	break;
	default:
		// There are no constructors left that could be called, so there must be an error.
		return luaL_error(L, "Too many arguments to quat()\n");
	}

	// Returning 1 value (the new vec4), already on the stack.
	return 1;
}


int lua_quat_tostring(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");
	char str[32];
	snprintf(str, 32, "{%f,%f,%f,%f}", self->x, self->y, self->z, self->w);
	lua_pushstring(L, str);
	return 1;
}

int lua_quat_add(lua_State* L)
{
	// vector addition can only be done between two vectors.

	// Get the arguments, make sure they're quat's.
	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
	quat* rhs = (quat*)luaL_checkudata(L, 2, "quat");

	// Allocate a new quat to store the result, and then put the result there. //
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	*result = *lhs + *rhs;
	return 1;
}

int lua_quat_sub(lua_State* L)
{
	// vector subtraction can only be done between two vectors.

	// Get the arguments, make sure they're quat's.
	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
	quat* rhs = (quat*)luaL_checkudata(L, 2, "quat");

	// Allocate a new quat to store the result, and then put the result there. //
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	*result = *lhs - *rhs;
	return 1;
}

int lua_quat_mul(lua_State* L)
{
	// quaternion multiplication could be 'number*quat', 'quat*number', 'quat*quat', or 'quat*vector'.  We need to support each case.

	if (lua_isnumber(L, 1)) // must be number * quat
	{
		quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
		luaL_getmetatable(L, "quat");  lua_setmetatable(L, -2);

		quat* arg = (quat*)luaL_checkudata(L, 2, "quat");
		*result = (*arg) * (float)lua_tonumber(L, 1);
		return 1;
	}
	else if (lua_isnumber(L, 2)) // must be quat * number
	{
		quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
		luaL_getmetatable(L, "quat");  lua_setmetatable(L, -2);

		quat* arg = (quat*)luaL_checkudata(L, 1, "quat");
		*result = (*arg) * (float)lua_tonumber(L, 2);
		return 1;
	}
	else // either quat * quat or quat * vec3
	{
		quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
		void* rhs = lua_touserdata(L, 2);

		lua_getmetatable(L, 2);
		luaL_getmetatable(L, "quat");
		if (lua_equal(L, -1, -2))
		{
			lua_pop(L, 2);
			quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
			luaL_getmetatable(L, "quat");  lua_setmetatable(L, -2);

			*result = (*lhs) * (*(quat*)rhs);
			return 1;
		}

		lua_pop(L, 1);
		luaL_getmetatable(L, "vec3");
		if (lua_equal(L, -1, -2))
		{
			lua_pop(L, 2);
			vec3* result = (vec3*)lua_newuserdata(L, sizeof(vec3));
			luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

			*result = (*lhs) * (*(vec3*)rhs);
			return 1;
		}

		lua_pop(L, 2);
		return luaL_error(L, "Invalid arguments to quat.mul(); expected (number, quat), (quat, number), (quat, quat) or (quat, vec3).");
	}
}

int lua_quat_div(lua_State* L)
{
	// quaternion division should be 'quat/number'.
	// 'quat/quat' is undefined.

	// Get the arguments, make sure they're quat's.
	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
	float rhs = (float)luaL_checknumber(L, 2);

	// Allocate a new quat to store the result.
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat");
	lua_setmetatable(L, -2);

	*result = *lhs / rhs;

	return 1;
}

int lua_quat_unm(lua_State* L)
{
	// unary minus.

	// Get the argument, make sure it's a quat.
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	// Allocate a new quat to store the result.
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat");
	lua_setmetatable(L, -2);

	*result = -*self;

	return 1;
}

int lua_quat_eq(lua_State* L)
{
	// boolean equals

	// Get the args, make sure they're quats.
	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
	quat* rhs = (quat*)luaL_checkudata(L, 2, "quat");

	if (*lhs == *rhs)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_quat_index(lua_State* L)
{
	// Handles accessing an 'unknown' value from a quat.
	// (table, key)
	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
	if (lua_isnumber(L, 2))
	{
		int key = (int)lua_tonumber(L, 2);
		if (key < 1 || key > 4)
		{
			return luaL_error(L, "Invalid key for quat; index out of bounds.\n");
		}

		lua_pushnumber(L, lhs->data[key-1]);
	}
	else if (lua_isstring(L, 2))
	{
		const char* key = lua_tostring(L, 2);

		// Check to make sure that the key doesn't already exist in the quat metatable.
		// If it does, then we're trying to call a function, not access variables.
		luaL_getmetatable(L, "quat"); // Push (quat:meta) onto the stack
		lua_pushstring(L, key); // Push (key) onto the stack
		lua_rawget(L, -2); // Pops (key) from the stack, and pushes (quat:meta.key) onto the stack.
		if (!lua_isnil(L, -1)) // If quat:meta.key is not nil...
		{
			lua_remove(L, -2); // Pop quat:meta from the stack.
			return 1; // Return quat:meta.key.
		}
		else
		{
			lua_pop(L, 2); // Clean up after ourselves.
		}

		int n = (int)strlen(key);
		switch (n)
		{
		case 1:
			switch (key[0])
			{
			case 'x':
				lua_pushnumber(L, lhs->x);
				break;
			case 'y':
				lua_pushnumber(L, lhs->y);
				break;
			case 'z':
				lua_pushnumber(L, lhs->z);
				break;
			case 'w':
				lua_pushnumber(L, lhs->w);
				break;
			default:
				return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
			}
			break;
		case 2:
		{
			vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
			luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

			for (int i = 0; i < 2; ++i)
			{
				switch (key[i])
				{
				case 'x':
					result->data[i] = lhs->x;
					break;
				case 'y':
					result->data[i] = lhs->y;
					break;
				case 'z':
					result->data[i] = lhs->z;
					break;
				case 'w':
					result->data[i] = lhs->z;
					break;
				default:
					return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
				}
			}
		}
		break;
		case 3:
		{
			vec3* result = (vec3*)lua_newuserdata(L, sizeof(vec3));
			luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

			for (int i = 0; i < 3; ++i)
			{
				switch (key[i])
				{
				case 'x':
					result->data[i] = lhs->x;
					break;
				case 'y':
					result->data[i] = lhs->y;
					break;
				case 'z':
					result->data[i] = lhs->z;
					break;
				case 'w':
					result->data[i] = lhs->z;
					break;
				default:
					return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
				}
			}
		}
		break;
		case 4:
		{
			vec4* result = (vec4*)lua_newuserdata(L, sizeof(vec4));
			luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

			for (int i = 0; i < 4; ++i)
			{
				switch (key[i])
				{
				case 'x':
					result->data[i] = lhs->x;
					break;
				case 'y':
					result->data[i] = lhs->y;
					break;
				case 'z':
					result->data[i] = lhs->z;
					break;
				case 'w':
					result->data[i] = lhs->z;
					break;
				default:
					return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
				}
			}
		}
		break;
		default:
			return luaL_error(L, "Invalid number of accessors for quat (expected 1-4).\n");
		}
	}
	else
	{
		return luaL_error(L, "Invalid key type for quat; expected string or number.\n");
	}

	return 1;
}

int lua_quat_newindex(lua_State* L)
{
	// Handles assigning data to an 'unknown' value in a quat.
	// (table, key, value)
	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");

	if (lua_isnumber(L, 2))
	{
		int key = (int)lua_tonumber(L, 2);
		if (key < 1 || key > 4)
		{
			return luaL_error(L, "Invalid key for quat; index out of bounds.\n");
		}

		lhs->data[key - 1] = (float)luaL_checknumber(L, 3);
	}
	else if (lua_isstring(L, 2))
	{
		const char* key = lua_tostring(L, 2);

		int n = (int)strlen(key);
		switch (n)
		{
		case 1:
			switch (key[0])
			{
			case 'x':
				lhs->x = (float)luaL_checknumber(L, 3);
				break;
			case 'y':
				lhs->y = (float)luaL_checknumber(L, 3);
				break;
			case 'z':
				lhs->z = (float)luaL_checknumber(L, 3);
				break;
			case 'w':
				lhs->w = (float)luaL_checknumber(L, 3);
				break;
			default:
				return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
			}
			break;
		case 2:
		{
			if (key[0] == key[1])
			{
				return luaL_error(L, "Cannot assign to multiple identical accessors.\n");
			}
			vec2* rhs = (vec2*)luaL_checkudata(L, 3, "vec2");
			for (int i = 0; i < 2; ++i)
			{
				switch (key[i])
				{
				case 'x':
					lhs->x = rhs->data[i];
					break;
				case 'y':
					lhs->y = rhs->data[i];
					break;
				case 'z':
					lhs->z = rhs->data[i];
					break;
				case 'w':
					lhs->w = rhs->data[i];
					break;
				default:
					return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
				}
			}
		}
		break;
		case 3:
		{
			if (key[0] == key[1] || key[1] == key[2] || key[0] == key[2])
			{
				return luaL_error(L, "Cannot assign to multiple identical accessors.\n");
			}
			vec3* rhs = (vec3*)luaL_checkudata(L, 3, "vec3");
			for (int i = 0; i < 3; ++i)
			{
				switch (key[i])
				{
				case 'x':
					lhs->x = rhs->data[i];
					break;
				case 'y':
					lhs->y = rhs->data[i];
					break;
				case 'z':
					lhs->z = rhs->data[i];
					break;
				case 'w':
					lhs->w = rhs->data[i];
					break;
				default:
					return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
				}
			}
		}
		break;
		case 4:
		{
			if (key[0] == key[1] || key[1] == key[2] || key[0] == key[2] || key[0] == key[3] || key[1] == key[3] || key[2] == key[3])
			{
				return luaL_error(L, "Cannot assign to multiple identical accessors.\n");
			}
			vec4* rhs = (vec4*)luaL_checkudata(L, 3, "vec4");
			for (int i = 0; i < 4; ++i)
			{
				switch (key[i])
				{
				case 'x':
					lhs->x = rhs->data[i];
					break;
				case 'y':
					lhs->y = rhs->data[i];
					break;
				case 'z':
					lhs->z = rhs->data[i];
					break;
				case 'w':
					lhs->w = rhs->data[i];
					break;
				default:
					return luaL_error(L, "Invalid accessor for quat; expected 'x/y/z/w'.\n");
				}
			}
		}
		break;
		default:
			return luaL_error(L, "Invalid number of accessors for quat (expected 1-4).\n");
		}
	}
	else
	{
		return luaL_error(L, "Invalid key type for quat; expected string or number.\n");
	}

	return 0;
}

int lua_quat_magnitude(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	float retval = self->magnitude();
	lua_pushnumber(L, retval);

	return 1;
}

int lua_quat_normalize(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	self->normalize();

	return 0;
}

int lua_quat_normalized(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	quat* retval = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	*retval = self->normalized();

	return 1;
}

int lua_quat_copy(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	quat* retval = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	*retval = *self;

	return 1;
}

int lua_quat_to_euler(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	vec3* retval = (vec3*)lua_newuserdata(L, sizeof(vec3));
	luaL_getmetatable(L, "vec3");  lua_setmetatable(L, -2);

	*retval = self->to_euler();

	return 1;
}

int lua_quat_from_euler(lua_State* L)
{
	quat* retval = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	if (lua_isnumber(L, 1))
	{
		float x = (float)luaL_checknumber(L, 1);
		float y = (float)luaL_checknumber(L, 2);
		float z = (float)luaL_checknumber(L, 3);
		*retval = quat::euler(x, y, z);
	}
	else if (lua_istable(L, 1))
	{
		vec3 args;
		lua_pushnumber(L, 1); lua_gettable(L, -2); args.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, -2); args.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, -2); args.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		*retval = quat::euler(args);
	}
	else
	{
		vec3* rhs = (vec3*)luaL_checkudata(L, 1, "vec3");
		*retval = quat::euler(*rhs);
	}

	return 1;
}

int lua_quat_to_axis_angle(lua_State* L)
{
	quat* self = (quat*)luaL_checkudata(L, 1, "quat");

	vec3* axis = (vec3*)lua_newuserdata(L, sizeof(vec3));
	luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

	vec4 temp = self->to_axis_angle();
	*axis = { temp.x, temp.y, temp.z };

	lua_pushnumber(L, temp.w);
	return 2;
}

int lua_quat_from_axis_angle(lua_State* L)
{
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	vec3* axis = (vec3*)luaL_checkudata(L, 1, "vec3");
	float angle = (float)luaL_checknumber(L, 2);

	*result = quat::axis_angle(*axis, angle);

	return 1;
}

int lua_quat_lerp(lua_State* L)
{
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	quat* lhs = (quat*)luaL_checkudata(L, 1, "quat");
	quat* rhs = (quat*)luaL_checkudata(L, 2, "quat");
	float t = (float)luaL_checknumber(L, 3);

	*result = quat::lerp(*lhs, *rhs, t);
	return 1;
}

int lua_quat_lookat(lua_State* L)
{
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	vec3* arg = (vec3*)luaL_checkudata(L, 1, "vec3");

	*result = quat::look_at(*arg);

	return 1;
}

int lua_quat_fromto(lua_State* L)
{
	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	vec3* lhs = (vec3*)luaL_checkudata(L, 1, "vec3");
	vec3* rhs = (vec3*)luaL_checkudata(L, 2, "vec3");

	*result = quat::from_to_rotation(*lhs, *rhs);

	return 1;
}

void InitLuaQuat()
{
	lua_State* L = lua::getLuaState();

	luaL_newmetatable(L, "quat"); // Creates a metatable for our "quat" type in the registry, and pushes it onto the stack.
	 lua_pushcfunction(L, lua_quat_add); lua_setfield(L, -2, "__add"); // Pushes 'lua_quat_add' onto the stack, then pops it and associates it with the "__add" field of the metatable.
	 lua_pushcfunction(L, lua_quat_sub); lua_setfield(L, -2, "__sub"); // Right now, the metatable is at "-2" (the bottom of the stack), and the c function is "-1" (the top of the stack).
	 lua_pushcfunction(L, lua_quat_mul); lua_setfield(L, -2, "__mul"); // lua_setfield pops the function off the stack after it's done, so the metatable will be on top again.
	 lua_pushcfunction(L, lua_quat_div); lua_setfield(L, -2, "__div");
	 lua_pushcfunction(L, lua_quat_unm); lua_setfield(L, -2, "__unm");
	 lua_pushcfunction(L, lua_quat_eq); lua_setfield(L, -2, "__eq");
	 lua_pushcfunction(L, lua_quat_tostring); lua_setfield(L, -2, "__tostring");
	 lua_pushcfunction(L, lua_quat_index);  lua_setfield(L, -2, "__index");
	 lua_pushcfunction(L, lua_quat_newindex); lua_setfield(L, -2, "__newindex");
	 lua_pushcfunction(L, lua_quat_magnitude); lua_setfield(L, -2, "magnitude");
	 lua_pushcfunction(L, lua_quat_normalize); lua_setfield(L, -2, "normalize");
	 lua_pushcfunction(L, lua_quat_normalized); lua_setfield(L, -2, "normalized");
	 lua_pushcfunction(L, lua_quat_copy); lua_setfield(L, -2, "copy");
	 lua_pushcfunction(L, lua_quat_to_euler); lua_setfield(L, -2, "to_euler");
	 lua_pushcfunction(L, lua_quat_to_axis_angle); lua_setfield(L, -2, "to_axis_angle");
 	 lua_pushstring(L, "quat"); lua_setfield(L, -2, "typename");
	lua_pop(L, 1); // Clean up the metatable left on the stack.

	lua_newtable(L); // Global "quat" table.
	 lua_pushcfunction(L, lua_quat_new); lua_setfield(L, -2, "new");
	 quat* val = NULL;
	 val = (quat*)lua_newuserdata(L, sizeof(quat)); luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2); *val = QUAT_ZERO; lua_setfield(L, -2, "zero"); // Const quat.zero
	 val = (quat*)lua_newuserdata(L, sizeof(quat)); luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2); *val = QUAT_IDENTITY; lua_setfield(L, -2, "identity"); // Const quat.identity
	 lua_pushcfunction(L, lua_quat_from_euler); lua_setfield(L, -2, "euler");
	 lua_pushcfunction(L, lua_quat_from_axis_angle); lua_setfield(L, -2, "axis_angle");
	 lua_pushcfunction(L, lua_quat_lerp); lua_setfield(L, -2, "lerp");
	 lua_pushcfunction(L, lua_quat_lookat); lua_setfield(L, -2, "look");
	 lua_pushcfunction(L, lua_quat_fromto); lua_setfield(L, -2, "from_to");
	lua_setglobal(L, "quat");

	lua::RunString("SANDBOX.quat = readonly(_G.quat)", nullptr);
}

} // namespace vmath