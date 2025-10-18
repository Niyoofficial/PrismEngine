#pragma once

#define IMGUI_USE_WCHAR32 1

#define IM_VEC2_CLASS_EXTRA                                                     \
        constexpr ImVec2(const glm::float2& f) : x(f.x), y(f.y) {}              \
        operator glm::float2() const { return glm::float2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                         \
        constexpr ImVec4(const glm::float4& f) : x(f.x), y(f.y), z(f.z), w(f.w) {}  \
        operator glm::float4() const { return glm::float4(x,y,z,w); }

#define IM_DEBUG_BREAK PE_ASSERT(false)
