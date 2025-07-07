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
    - Add retries if queue is full (MVP)
    - Add timeout for a task. Each task can have expected time budget (MVP)
    - Remove all occurrences of dynamic_cast (MVP)
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
- SoC testing
    - Investigate which board is best for testing (MVP)