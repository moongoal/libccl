# Ccl C++ base library

## API stability

Stable: 🟢
Almost stable: 🟡
Unstable: 🔴

|Interface|Status
|-|-
|Utilities|🔴
|Vector|🔴
|Test Driver|🟡
|Maybe|🔴
|Hashtable|🔴
|Hash|🟡
|Debug Utilities|🔴
|Compressed Pair|🔴
|Bitset|🔴
|Allocators|🔴
|Tables|🔴
|ECS|🔴
|Dense Map|🔴
|Handle|🔴
|Packed Integer|🔴
|Paged Vector|🔴
|Set|🔴
|Sparse Set|🔴
|Tagged pointer|🔴

## Dependencies

### Build dependencies

* Cmake `>= 3.24.2`
* LLVM `>= 15.0.0`
* Ninja `>= 1.11.0`
* Conan `>= 1.55.0`

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

## Packaging

This library can be packaged with Conan, by running:

```
    conan create -pr:b <YOUR_PROFILE> .
```

## Testing

To run the tests:

```
ctest --test-dir build
```

## Development

### Clangd

This project is configured for clangd. Run as follows:

```
clangd --header-insertion=never --compile-commands-dir=build --enable-config
```
