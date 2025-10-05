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
        - clang check
            ```
                    📊 Clang-Tidy Warning Summary for temp/tidy.log
            -------------------------------------------------
            Total warnings (lines with 'warning:'): 507

            Per-check counts:
            -----------------
            cppcoreguidelines-avoid-magic-numbers         98
            readability-magic-numbers                     98
            readability-identifier-length                 83
            cppcoreguidelines-pro-bounds-constant-array-index 58
            nodiscard                                     46
            modernize-concat-nested-namespaces            32
            cppcoreguidelines-special-member-functions    31
            cppcoreguidelines-pro-type-member-init        24
            clang-analyzer-deadcode.DeadStores            11
            readability-uppercase-literal-suffix          11
            cppcoreguidelines-non-private-member-variables-in-classes 10
            cppcoreguidelines-prefer-member-initializer   10
            cppcoreguidelines-pro-type-reinterpret-cast   10
            cppcoreguidelines-avoid-non-const-global-variables 9
            cppcoreguidelines-explicit-virtual-functions  8
            modernize-return-braced-init-list             8
            modernize-use-override                        8
            cppcoreguidelines-avoid-c-arrays              7
            cppcoreguidelines-pro-bounds-array-to-pointer-decay 7
            modernize-avoid-c-arrays                      7
            modernize-use-equals-default                  7
            readability-make-member-function-const        6
            readability-qualified-auto                    5
            bugprone-easily-swappable-parameters          4
            cppcoreguidelines-owning-memory               4
            performance-unnecessary-value-param           4
            performance-noexcept-move-constructor         3
            readability-named-parameter                   3
            clang-analyzer-core.NonNullParamChecker       2
            cppcoreguidelines-pro-type-static-cast-downcast 2
            modernize-use-auto                            2
            readability-duplicate-include                 1
            readability-function-cognitive-complexity     1
            -------------------------------------------------
            Tip: disable specific checks in your .clang-tidy via:
                  Checks: '-check-name'
            ```
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
 


