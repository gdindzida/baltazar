#ifndef BALTAZAR_CORE_PARALLEL_API
#define BALTAZAR_CORE_PARALLEL_API

#include "../../src/core/core_parallel.hpp"
#include "../../src/core/multithreaded_profiling.hpp"

namespace baltazar {

template <typename PROFILER_TYPE = core::NullProfiler>
using ParallelCoreRunner = core::ParallelCoreRunner<PROFILER_TYPE>;

template <size_t QUEUE_SIZE>
using MultiThreadedCoreProfiler = core::MultiThreadedCoreProfiler<QUEUE_SIZE>;

} // namespace baltazar

#endif
