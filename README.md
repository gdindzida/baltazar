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
            Total warnings (lines with 'warning:'): 837
            
            Per-check counts:
            -----------------
            readability-identifier-length                 129
            cppcoreguidelines-avoid-magic-numbers         104
            readability-magic-numbers                     104
            nodiscard                                     90
            cppcoreguidelines-macro-usage                 76
            cppcoreguidelines-pro-bounds-constant-array-index 58
            modernize-use-default-member-init             46
            cppcoreguidelines-special-member-functions    43
            modernize-concat-nested-namespaces            32
            cppcoreguidelines-pro-type-member-init        26
            modernize-use-using                           24
            cppcoreguidelines-non-private-member-variables-in-classes 22
            cppcoreguidelines-avoid-non-const-global-variables 13
            bugprone-macro-parentheses                    12
            modernize-return-braced-init-list             12
            clang-analyzer-deadcode.DeadStores            11
            readability-uppercase-literal-suffix          11
            cppcoreguidelines-prefer-member-initializer   10
            cppcoreguidelines-pro-type-reinterpret-cast   10
            cppcoreguidelines-avoid-c-arrays              9
            modernize-avoid-c-arrays                      9
            modernize-use-equals-default                  9
            readability-named-parameter                   9
            cppcoreguidelines-explicit-virtual-functions  8
            cppcoreguidelines-owning-memory               8
            modernize-use-override                        8
            cppcoreguidelines-pro-bounds-array-to-pointer-decay 7
            bugprone-easily-swappable-parameters          6
            modernize-use-equals-delete                   6
            readability-braces-around-statements          6
            readability-make-member-function-const        6
            readability-qualified-auto                    5
            cppcoreguidelines-pro-type-const-cast         4
            modernize-use-emplace                         4
            performance-unnecessary-value-param           4
            performance-noexcept-move-constructor         3
            readability-duplicate-include                 3
            clang-analyzer-core.NonNullParamChecker       2
            cppcoreguidelines-pro-type-static-cast-downcast 2
            modernize-deprecated-headers                  2
            modernize-pass-by-value                       2
            modernize-use-nullptr                         2
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
 


