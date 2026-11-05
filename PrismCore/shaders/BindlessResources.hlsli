#ifndef BINDLESS_HLSLI
#define BINDLESS_HLSLI

#ifdef VULKAN
    [[vk::binding(9, 0)]]
    ByteAddressBuffer g_bindlessBuffers[] : register(t0, space100);

    #define GET_BINDLESS_UNIFORM_BUFFER(StructType, index) g_bindlessBuffers[index].Load<StructType>(0)
#else

    // ResourceDescriptorHeap returns rvalue of type `const .Resource`, so we add implicit casting to T to support Vulkan and DX with same HLSL code
    template<typename T>
    T GetBindlessConstantBuffer(uint index)
    {
        ConstantBuffer<T> buffer = ResourceDescriptorHeap[index];
        return buffer;
    }

    #define GET_BINDLESS_UNIFORM_BUFFER(StructType, index) GetBindlessConstantBuffer<StructType>(index)
#endif

#endif
