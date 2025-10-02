# baltazar

![Not sure about this logo but it is ok for now :)](assets/logo.png)

Baltazar is lightweight data processing pipeline. Each task is represented as node in directed acyclic graph (DAG) which
can be executed in parallel by the thread pool or in sequence. Sequence of execution/scheduling is determined by
topological sort of DAG.

## Roadmap

### How to build, run & debug

-- TODO: instructions for baltazar controller
-- TODO: setup neovim debug


### Releases

- MVP ~ oct - dec???
- 1.0 - ???

### Features

- Features
    - Implement abstraction for threads:
        - implement rtos threads
            - qnx
    - Add timeout for a task. Each task can have expected time budget
    - Clang-Tidy: Add checks like cppcoreguidelines-no-malloc, hicpp-no-exceptions, etc.  (MVP)
        - For embedded complience
    - GPU support for CUDA and openCL
        - What is needed to make it easy to use?
- Support
    - DAG visualization (MVP)
    - investigate creating DAG input as yaml or json or something similar
    - 2D plot visualizer plugin
    - 3D plot visualizer plugin
    - Image visualizer plugin (MVP)
    - Investigate how to make debugging easy (MVP)
    - Make code MISRA compliant
- Tests & Benchmarks
    - test benchmarks (MVP)
- Plugins
    - Implement RL reverse pendulum stabilizer
    - MVP example??? (MVP)
- Bugs
- CI/CD
    - Setup basic git action checks (MVP)
        - all tests
            - Windows testing
            - Linux testing
        - all clang-tidy and format checks
        - investigate how to test benchmarks
- SoC testing
    - How to cross compile?
    - Investigate which board is best for testing (MVP)
        - RPI (MVP)
        - STM32 (MVP)
            - which one?
    - setup HiLs
        - setup local server
