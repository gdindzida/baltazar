#ifndef CORE_H
#define CORE_H
#include "../dag/dag.h"
#include <atomic>

namespace core {
// TODO add retries if cant schedule a task
// TODO add timeout for a task, like expected time budget
// TODO finish tests
// TODO create benchmark
template <size_t GRAPH_SIZE>
void runGraphSerialOnce(dag::Dag<GRAPH_SIZE> &graph,
                        std::atomic<bool> &stopFlag);

template <size_t GRAPH_SIZE>
void runGraphSerialNTimes(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag, size_t n);

template <size_t GRAPH_SIZE>
void runGraphSerialLoop(dag::Dag<GRAPH_SIZE> &graph,
                        std::atomic<bool> &stopFlag);

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelOnce(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag);

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelNTimes(dag::Dag<GRAPH_SIZE> &graph,
                            std::atomic<bool> &stopFlag, size_t n,
                            bool waitForPrevious);

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelLoop(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag, bool waitForPrevious);

} // namespace core

#endif // CORE_H
