#ifndef HVH_WC_TOOLS_COLORS_H
#define HVH_WC_TOOLS_COLORS_H

#include "math/vmath.h"

namespace colors {

bool is_valid(const char* name);
bool get(const char* name, vmath::vec3& color);
bool get(const char* name, vmath::vec4& color);
bool parse(const char* str, vmath::vec3& color);
bool parse(const char* str, vmath::vec4& color);

} // namespace colors

#endif // HVH_WC_TOOLS_COLORS_H