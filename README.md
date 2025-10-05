# baltazar

![Not sure about this logo but it is ok for now :)](assets/logo.png)

Baltazar is lightweight data processing pipeline. Each task is represented as node in directed acyclic graph (DAG) which
can be executed in parallel by the thread pool or in sequence. Sequence of execution/scheduling is determined by
topological sort of DAG.

## Roadmap

### How to build, run & debug

-- TODO


### Releases

- MVP ~ oct - dec???
- 1.0 - ???

### Work packages

- MVP
    - CI/CD
        - clang checks
        - github actions
        - test benchmarks
    - Docs
        - doxygen comments for the api
        - build docs
        - profiling docs
        - github readme
    - Test
        - investigate how to test
        - select platforms to test
        - run build, test, benchmarks and examples on each 
    - DAG visualisation 
        - implement read and write to yaml functions 
        - write python script for yaml visualisation 

- Planned
    - Python api
        - implement dynamic node
        - pybind
        - setup pybind build with default and custom size parameters
    - QNX & FreeRtos
        - investigate how to build for them
        - ...?
    - Acceleration 
        - investigate how to add gpu accelerated support 
        - ...?
 


