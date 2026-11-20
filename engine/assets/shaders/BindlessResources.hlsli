#ifndef BINDLESS_HLSLI
#define BINDLESS_HLSLI

#ifdef VULKAN
    #define RESOURCE_INDICES(StructName, ...) \
            struct StructName { __VA_ARGS__ }; \
            [[vk::push_constant]] StructName Resources
#else
    #define RESOURCE_INDICES(StructName, ...) \
           struct StructName { __VA_ARGS__ }; \
           cbuffer _ResourcesCB : register(b0, space0) { StructName Resources; }
#endif

#endif
