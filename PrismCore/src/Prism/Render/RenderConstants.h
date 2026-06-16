#pragma once

namespace Prism::Render::Constants
{
constexpr int32_t MAX_FRAMES_IN_FLIGHT = 3;

constexpr int32_t DESCRIPTOR_COUNT_PER_CPU_HEAP = 1024;
constexpr int32_t DESCRIPTOR_COUNT_PER_GPU_HEAP = 65536;
constexpr int32_t DESCRIPTOR_COUNT_PER_GPU_SAMPLER_HEAP = 1024;

constexpr int32_t UNIFORM_BUFFER_ALIGNMENT = 256;
}
