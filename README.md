# baltazar

Baltazar is lightweight data processing pipeline. Each task is represented as node in directed acyclic graph (DAG) which
can be executed in parallel by the thread pool or in sequence. Sequence of execution/scheduling is determined by
topological sort of DAG.

## Roadmap

### Releases

- MVP ~ oct - dec???
- 1.0 - ???

### Features

- Features
    - Implement abstraction for threads:  (MVP)
        - implement rtos threads
    - Add retries if queue is full (MVP)
    - Add timeout for a task. Each task can have expected time budget (MVP)
    - Implement thread priorities (MVP)
    - Remove all occurrences of dynamic_cast (MVP)
    - Clang-Tidy: Add checks like cppcoreguidelines-no-malloc, hicpp-no-exceptions, etc.  (MVP)
        - For embedded complience
    - GPU support for CUDA and openCL
        - What is needed to make it easy to use?
    - Investigate lock-free structures and implement queue as one (MVP)
    - Read thread pool chapter in C++ concurrency in action and adapt baltazar (MVP)
    - ??? (MVP)
- Support
    - DAG visualization (MVP)
    - investigate creating DAG input as yaml or json or something similar
    - 2D plot visualizer plugin (MVP)
    - 3D plot visualizer plugin (MVP)
    - Image visualizer plugin (MVP)
    - Investigate how to make debugging easy (MVP)
    - Investigate how to debug thread pool (MVP)
    - Investigate how to profile pipeline easily (MVP)
- Tests & Benchmarks
    - Finish core tests (MVP)
    - Create core benchmark (MVP)
- Plugins
    - CV detection pipeline (MVP)
    - ROS robot manipulator pipeline
    - ???
- Bugs
    - ???
- CI/CD
    - Setup basic git action checks (MVP)
        - all tests
            - Windows testing
            - Linux testing
        - all clang-tidy and format checks
        - investigate how to test benchmarks
- SoC testing
    - Investigate which board is best for testing (MVP)
        - Memory constraints
            - Is it ok for user to provide all the memory thus keeping framework flexible
            - Is this too complex? How to make it simple to use?
        - RPI
        - STM32 (MVP)
            - which one?
    - setup HiLs
        - remote server???
        - local server?