#ifndef BINDLESS_HLSLI
#define BINDLESS_HLSLI

#ifdef VULKAN
    [[vk::binding(2, 0)]]
    ByteAddressBuffer g_bindlessBuffers[] : register(t0, space100);

    #define GET_BINDLESS_CBUFFER(StructType, index) g_bindlessBuffers[index].Load<StructType>(0)
#else
    #define GET_BINDLESS_CBUFFER(StructType, index) ((ConstantBuffer<StructType>)ResourceDescriptorHeap[index])
#endif

#endif
