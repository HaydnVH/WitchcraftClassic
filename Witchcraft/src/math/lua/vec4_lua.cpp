#include "vmath_lua.h"

#include "../vec2.h"
#include "../vec3.h"
#include "../vec4.h"

#include "sys/printlog.h"
#include "scripting/luasystem.h"

namespace vmath {

int lua_vec4_new(lua_State* L)
{
	// If the first argument on the stack is a table, then we're creating a vec4 using "var = vec4()".
	// If it isn't, then we're creating it using "var = vec4.new()".
	if (lua_istable(L, 1))
	{
		// If the first arg is the table, let's just remove it.
		lua_remove(L, 1);
	}

	// Get the number of arguments on the stack.  Should be 0, 1, 2, or 3.
	int argc = lua_gettop(L);

	// Allocate space for the new vec4 within lua.
	vec4* self = (vec4*)lua_newuserdata(L, sizeof(vec4));
	// Associate the "vec4" metatable with the value we just created.
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	switch (argc)
	{
	case 0: // 0 arguments, therefore default constructor.
		*self = { 0, 0, 0, 0 };
		break;
	case 1: // 1 argument, should be a vec4.
	{
		vec4* rhs = (vec4*)luaL_checkudata(L, 1, "vec4");
		*self = *rhs;
	}
	break;
	case 2: // 2 arguments, could be (vec2, vec2), (vec3, number), or (number, vec3).
	{
		if (lua_isnumber(L, 1)) // number, vec3
		{
			self->x = (float)lua_tonumber(L, 1);
			self->yzw = *(vec3*)luaL_checkudata(L, 2, "vec3");
		}
		else if (lua_isnumber(L, 2)) // vec3, number
		{
			self->xyz = *(vec3*)luaL_checkudata(L, 1, "vec3");
			self->w = (float)lua_tonumber(L, 2);
		}
		else // vec2, vec2
		{
			self->xy = *(vec2*)luaL_checkudata(L, 1, "vec2");
			self->zw = *(vec2*)luaL_checkudata(L, 2, "vec2");
		}
	}
	break;
	case 3: // 3 arguments, could be (vec2, number, number), (number, vec2, number), or (number, number, vec2).
	{
		if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) // number, number, vec2
		{
			self->x = (float)lua_tonumber(L, 1);
			self->y = (float)lua_tonumber(L, 2);
			self->xy = *(vec2*)luaL_checkudata(L, 3, "vec2");
		}
		else if (lua_isnumber(L, 1) && lua_isnumber(L, 3)) // number, vec2, number
		{
			self->x = (float)lua_tonumber(L, 1);
			self->yz = *(vec2*)luaL_checkudata(L, 2, "vec2");
			self->w = (float)lua_tonumber(L, 3);
		}
		else if (lua_isnumber(L, 2) && lua_isnumber(L, 3)) // vec2, number, number
		{
			self->xy = *(vec2*)luaL_checkudata(L, 1, "vec2");
			self->z = (float)lua_tonumber(L, 2);
			self->w = (float)lua_tonumber(L, 3);
		}
		else
		{
			// Error
			return luaL_error(L, "Bad arguments to 'vec4.new()'; expected (vec2, number, number), (number, vec2, number) or (number, number, vec2).\n");
		}
	}	
		break;
	case 4: // 4 arguments, should all be numbers.
	{
		self->x = (float)luaL_checknumber(L, 1);
		self->y = (float)luaL_checknumber(L, 2);
		self->z = (float)luaL_checknumber(L, 3);
		self->w = (float)luaL_checknumber(L, 4);
	}
	break;
	default:
		// There are no constructors left that could be called, so there must be an error.
		return luaL_error(L, "Too many arguments to vec4.new()\n");
	}

	// Returning 1 value (the new vec4), already on the stack.
	return 1;
}

int lua_vec4_tostring(lua_State* L)
{
	vec4* self = (vec4*)luaL_checkudata(L, 1, "vec4");
	char str[32];
	snprintf(str, 32, "{%f,%f,%f,%f}", self->x, self->y, self->z, self->w);
	lua_pushstring(L, str);
	return 1;
}

int lua_vec4_add(lua_State* L)
{
	// vector addition can only be done between two vectors.

	// Get the arguments, make sure they're vec4's.
	vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");
	vec4* rhs = (vec4*)luaL_checkudata(L, 2, "vec4");

	// Allocate a new vec4 to store the result, and then put the result there. //
	vec4* result = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	*result = *lhs + *rhs;
	return 1;
}

int lua_vec4_sub(lua_State* L)
{
	// vector subtraction can only be done between two vectors.

	// Get the arguments, make sure they're vec4's.
	vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");
	vec4* rhs = (vec4*)luaL_checkudata(L, 2, "vec4");

	// Allocate a new vec4 to store the result, and then put the result there. //
	vec4* result = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	*result = *lhs - *rhs;
	return 1;
}

int lua_vec4_mul(lua_State* L)
{
	// vector multiplication could be "number * vector", "vector * number", or "vector * vector".  We need to support each case.
	bool arg1num = (lua_isnumber(L, 1) != 0);
	bool arg2num = (lua_isnumber(L, 2) != 0);
	if (arg1num && arg2num)
	{
		return luaL_error(L, "Invalid arguments passed to vec4.mul.  Expected (vec4, vec4), (vec4, number), or (number, vec4).\n");
	}

	// Allocate a new vec4 to store the result, and then put the result there.
	vec4* result = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	if (arg1num) // number * vec4
	{
		float lhs = (float)lua_tonumber(L, 1);
		vec4* rhs = (vec4*)luaL_checkudata(L, 2, "vec4");
		*result = lhs * (*rhs);
	}
	else if (arg2num) // vec4 * number
	{
		vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");
		float rhs = (float)lua_tonumber(L, 2);
		*result = (*lhs) * rhs;
	}
	else // vec4 * vec4
	{
		vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");
		vec4* rhs = (vec4*)luaL_checkudata(L, 2, "vec4");
		*result = (*lhs) * (*rhs);
	}

	return 1;
}

int lua_vec4_div(lua_State* L)
{
	// vector division could be "vector / number", or "vector / vector".  We need to support each case.

	// Get the arguments, make sure they're vec4's.
	vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");

	// Are we dividing by a number?
	bool divnum = (lua_isnumber(L, 2) != 0);

	// Allocate a new vec4 to store the result.
	vec4* result = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	if (divnum)
	{
		float rhs = (float)lua_tonumber(L, 2);
		*result = *lhs / rhs;
	}
	else
	{
		vec4* rhs = (vec4*)luaL_checkudata(L, 2, "vec4");
		*result = *lhs / *rhs;
	}

	return 1;
}

int lua_vec4_unm(lua_State* L)
{
	// unary minus.

	// Get the argument, make sure it's a vec4.
	vec4* self = (vec4*)luaL_checkudata(L, 1, "vec4");

	// Allocate a new vec4 to store the result.
	vec4* result = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4");
	lua_setmetatable(L, -2);

	*result = -*self;

	return 1;
}

int lua_vec4_eq(lua_State* L)
{
	// boolean equals

	// Get the args, make sure they're vec4s.
	vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");
	vec4* rhs = (vec4*)luaL_checkudata(L, 2, "vec4");

	if (*lhs == *rhs)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_vec4_index(lua_State* L)
{
	// Handles accessing an 'unknown' value from a vec4.
	// (table, key)
	vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");

	if (lua_isnumber(L, 2))
	{
		int key = (int)lua_tonumber(L, 2);
		if (key < 1 || key > 4)
		{
			return luaL_error(L, "Invalid key for vec4; index out of bounds.\n");
		}

		lua_pushnumber(L, lhs->data[key - 1]);
	}
	else if (lua_isstring(L, 2))
	{
		const char* key = lua_tostring(L, 2);

		// Check to make sure that the key doesn't already exist in the vec4 metatable.
		// If it does, then we're trying to call a function, not access variables.
		luaL_getmetatable(L, "vec4"); // Push (vec4:meta) onto the stack
		lua_pushstring(L, key); // Push (key) onto the stack
		lua_rawget(L, -2); // Pops (key) from the stack, and pushes (vec4:meta.key) onto the stack.
		if (!lua_isnil(L, -1)) // If vec4:meta.key is not nil...
		{
			lua_remove(L, -2); // Pop vec4:meta from the stack.
			return 1; // Return vec4:meta.key.
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
			case 'r':
				lua_pushnumber(L, lhs->r);
				break;
			case 'g':
				lua_pushnumber(L, lhs->g);
				break;
			case 'b':
				lua_pushnumber(L, lhs->b);
				break;
			case 'a':
				lua_pushnumber(L, lhs->a);
				break;
			default:
				return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
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
				case 'r':
					result->data[i] = lhs->r;
					break;
				case 'g':
					result->data[i] = lhs->g;
					break;
				case 'b':
					result->data[i] = lhs->b;
					break;
				case 'a':
					result->data[i] = lhs->a;
					break;
				default:
					return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
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
				case 'r':
					result->data[i] = lhs->r;
					break;
				case 'g':
					result->data[i] = lhs->g;
					break;
				case 'b':
					result->data[i] = lhs->b;
					break;
				case 'a':
					result->data[i] = lhs->a;
					break;
				default:
					return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
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
				case 'r':
					result->data[i] = lhs->r;
					break;
				case 'g':
					result->data[i] = lhs->g;
					break;
				case 'b':
					result->data[i] = lhs->b;
					break;
				case 'a':
					result->data[i] = lhs->a;
					break;
				default:
					return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
				}
			}
		}
		break;
		default:
			return luaL_error(L, "Invalid key type for vec4; expected a number or 1-4 characters (x/y/z/w).\n");
		}
	}
	else
	{
		return luaL_error(L, "Invalid key type for vec4; expected string or number.\n");
	}

	return 1;
}

int lua_vec4_newindex(lua_State* L)
{
	// Handles assigning data to an 'unknown' value in a vec4.
	// (table, key, value)
	vec4* lhs = (vec4*)luaL_checkudata(L, 1, "vec4");

	if (lua_isnumber(L, 2))
	{
		int key = (int)lua_tonumber(L, 2);
		if (key < 1 || key > 4)
		{
			return luaL_error(L, "Invalid key for vec4; index out of bounds.\n");
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
			case 'r':
				lhs->r = (float)luaL_checknumber(L, 3);
				break;
			case 'g':
				lhs->g = (float)luaL_checknumber(L, 3);
				break;
			case 'b':
				lhs->b = (float)luaL_checknumber(L, 3);
				break;
			case 'a':
				lhs->a = (float)luaL_checknumber(L, 3);
				break;
			default:
				return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
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
				case 'r':
					lhs->r = rhs->data[i];
					break;
				case 'g':
					lhs->g = rhs->data[i];
					break;
				case 'b':
					lhs->b = rhs->data[i];
					break;
				case 'a':
					lhs->a = rhs->data[i];
					break;
				default:
					return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
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
				case 'r':
					lhs->r = rhs->data[i];
					break;
				case 'g':
					lhs->g = rhs->data[i];
					break;
				case 'b':
					lhs->b = rhs->data[i];
					break;
				case 'a':
					lhs->a = rhs->data[i];
					break;
				default:
					return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
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
				case 'r':
					lhs->r = rhs->data[i];
					break;
				case 'g':
					lhs->g = rhs->data[i];
					break;
				case 'b':
					lhs->b = rhs->data[i];
					break;
				case 'a':
					lhs->a = rhs->data[i];
					break;
				default:
					return luaL_error(L, "Invalid accessor for vec4; expected 'x/y/z/w' or 'r/g/b/a'.\n");
				}
			}
		}
		break;
		default:
			return luaL_error(L, "Invalid number of accessors for vec4 (expected 1-4).\n");
		}
	}
	else
	{
		return luaL_error(L, "Invalid key type for vec4; expected string or number.\n");
	}

	return 0;
}

int lua_vec4_magnitude(lua_State* L)
{
	vec4* self = (vec4*)luaL_checkudata(L, 1, "vec4");

	float retval = self->magnitude();
	lua_pushnumber(L, retval);

	return 1;
}

int lua_vec4_normalize(lua_State* L)
{
	vec4* self = (vec4*)luaL_checkudata(L, 1, "vec4");

	self->normalize();

	return 0;
}

int lua_vec4_normalized(lua_State* L)
{
	vec4* self = (vec4*)luaL_checkudata(L, 1, "vec4");

	vec4* retval = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	*retval = self->normalized();

	return 1;
}

int lua_vec4_copy(lua_State* L)
{
	vec4* self = (vec4*)luaL_checkudata(L, 1, "vec4");

	vec4* retval = (vec4*)lua_newuserdata(L, sizeof(vec4));
	luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2);

	*retval = *self;

	return 1;
}

void InitLuaVec4()
{
	lua_State* L = lua::getLuaState();

	luaL_newmetatable(L, "vec4"); // Creates a metatable for our "vec4" type in the registry, and pushes it onto the stack.
	 lua_pushcfunction(L, lua_vec4_add); lua_setfield(L, -2, "__add"); // Pushes 'lua_vec4_add' onto the stack, then pops it and associates it with the "__add" field of the metatable.
	 lua_pushcfunction(L, lua_vec4_sub); lua_setfield(L, -2, "__sub"); // Right now, the metatable is at "-2" (the bottom of the stack), and the c function is "-1" (the top of the stack).
	 lua_pushcfunction(L, lua_vec4_mul); lua_setfield(L, -2, "__mul"); // lua_setfield pops the function off the stack after it's done, so the metatable will be on top again.
	 lua_pushcfunction(L, lua_vec4_div); lua_setfield(L, -2, "__div");
	 lua_pushcfunction(L, lua_vec4_unm); lua_setfield(L, -2, "__unm");
	 lua_pushcfunction(L, lua_vec4_eq); lua_setfield(L, -2, "__eq");
	 lua_pushcfunction(L, lua_vec4_tostring); lua_setfield(L, -2, "__tostring");
	 lua_pushcfunction(L, lua_vec4_index);  lua_setfield(L, -2, "__index");
	 lua_pushcfunction(L, lua_vec4_newindex); lua_setfield(L, -2, "__newindex");
	 lua_pushcfunction(L, lua_vec4_magnitude); lua_setfield(L, -2, "magnitude");
	 lua_pushcfunction(L, lua_vec4_normalize); lua_setfield(L, -2, "normalize");
	 lua_pushcfunction(L, lua_vec4_normalized); lua_setfield(L, -2, "normalized");
	 lua_pushcfunction(L, lua_vec4_copy); lua_setfield(L, -2, "copy");
 	 lua_pushstring(L, "vec4"); lua_setfield(L, -2, "typename");
	lua_pop(L, 1); // Clean up the metatable left on the stack.

	lua_newtable(L); // Global "vec4" table.
	 lua_pushcfunction(L, lua_vec4_new); lua_setfield(L, -2, "new");
	 vec4* val = NULL;
	 val = (vec4*)lua_newuserdata(L, sizeof(vec4)); luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2); *val = VEC4_ZERO; lua_setfield(L, -2, "zero"); // Const vec4.zero
	 val = (vec4*)lua_newuserdata(L, sizeof(vec4)); luaL_getmetatable(L, "vec4"); lua_setmetatable(L, -2); *val = VEC4_ONE; lua_setfield(L, -2, "one"); // Const vec4.one
	lua_setglobal(L, "vec4");

	lua::RunString("SANDBOX.vec4 = readonly(_G.vec4)", nullptr);
}

} // namespace vmath