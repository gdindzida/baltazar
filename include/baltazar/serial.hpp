#ifndef BALTAZAR_SERIAL_CORE_API
#define BALTAZAR_SERIAL_CORE_API

#include "../../src/core/core_serial.hpp"

namespace baltazar {

template <typename PROFILER_TYPE = core::NullProfiler>
using SerialCoreRunner = core::SerialCoreRunner<PROFILER_TYPE>;

template <size_t QUEUE_SIZE>
using SingleThreadedCoreProfiler = core::SingleThreadedCoreProfiler<QUEUE_SIZE>;

} // namespace baltazar

#endif
