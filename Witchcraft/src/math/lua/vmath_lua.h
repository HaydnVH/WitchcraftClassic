#ifndef HVH_VMATH_VEC2_LUA_H
#define HVH_VMATH_VEC2_LUA_H

#include "math/vmath.h"

namespace vmath {

void InitLuaVec2();
void InitLuaVec3();
void InitLuaVec4();
void InitLuaQuat();
void InitLuaMat4();

/* Initializes the Lua interface for various vmath types. */
/* Must initialize after: Lua. */
inline void InitLua()
{
	InitLuaVec2();
	InitLuaVec3();
	InitLuaVec4();
	InitLuaQuat();
	//InitLuaMat4();
}

} // namespace vmath

#endif