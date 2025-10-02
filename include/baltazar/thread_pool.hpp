#ifndef BALTAZAR_THREAD_POOL_API
#define BALTAZAR_THREAD_POOL_API

#include "../../src/thread_pool/thread_pool.hpp"
#include "../../src/thread_pool/thread_task.hpp"

namespace baltazar {

using IThreadTask = threadPool::IThreadTask;

template <size_t THREAD_NUM, size_t MAX_QUEUE_SIZE>
using ThreadPool = threadPool::ThreadPool<THREAD_NUM, MAX_QUEUE_SIZE>;

} // namespace baltazar

#endif
