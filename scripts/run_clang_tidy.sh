run-clang-tidy -p build-debug $(find src -name '*.cpp')   | grep -vE 'Suppressed|warnings generated|Use -header-filter'   | tee temp/tidy.log   | grep -c "warning:"
