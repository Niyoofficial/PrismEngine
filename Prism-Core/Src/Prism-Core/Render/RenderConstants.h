#pragma once

namespace Prism::Render::Constants
{
constexpr uint32_t RENDER_THREAD_CMD_LIST_BYTE_SIZE = 1024 * 1024 * 10; // 10mb

constexpr int32_t RENDER_THREADS_COUNT = 3;

constexpr int32_t DESCRIPTOR_COUNT_PER_CPU_HEAP = 1024;
}
