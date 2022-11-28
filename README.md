# Ccl C++ base library

## Dependencies

**Build dependencies**

* Cmake `>= 3.24.2`
* LLVM `>= 15.0.0`
* Ninja `>= 1.11.0`

**Runtime dependencies**

* XXHash `>= 0.8.1`

## Building

This library is built with cmake `>= 3.24.2`. Lower versions may work but are not supported. The following are example build commands for informational purposes only. The specific commands for your system may differ.

To initialise the build system for development, run:

```
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -DCMAKE_BUILD_TYPE=Debug
```

Or with `-DCMAKE_BUILD_TYPE=Release` to use the release configuration.

To build the project:

```
cmake --build build
```

## Testing

To run the tests:

```
ctest --test-dir build
```
