# CCL C++ base library

## API stability

API and implementation stable: 🟢

API stable: 🟡

Unstable: 🔴

|Interface|Status
|-|-
|Utilities|🔴
|Vector|🔴
|Ring|🔴
|Test Driver|🟡
|Maybe|🔴
|Hashtable|🔴
|Hash|🔴
|Debug Utilities|🔴
|Compressed Pair|🔴
|Bitset|🔴
|Allocators|🔴
|Dense Map|🔴
|Packed Integer|🔴
|Paged Vector|🔴
|Pool|🔴
|Set|🔴
|Sparse Set|🔴
|Tagged pointer|🔴
|Pair|🔴
|Deque|🔴
|Shared Pointer|🔴
|String|🔴
|Internationalization Support|🔴
|Thread communication primitives|🔴
|Algorithms library|🔴

## Building

While this is a header-only library, the build process is required to create the *version.hpp* header
file.

This library is built with CMake `>= 3.24.2`. Lower versions may work but are not supported. The following are example build commands for informational purposes only. The specific commands for your system may differ.

To initialise the build system for development, run:

```
pipenv install -d
pipenv run conan install --build=missing --deployer=full_deploy -s build_type=Debug .
```

To configure the build, run:

```
build/Debug/generators/conanbuild.bat
cmake --preset dev
```

Or with `--preset release` to use the release configuration.

To enable test coverage data to be collected, append `-DCCL_COVERAGE:BOOL=ON` - enabled by default for the *dev* pre-set. This will enable instrumentation of tests in order to collect coverage data, the `coverage-summary` and `coverage-report` build targets.

Similarly, to build the project:

```
build/Debug/generators/conanbuild.bat
cmake --build --preset dev
```

### Build dependencies

These dependencies must be manually installed on the system:

* Python `>= 3.12`
* LLVM `>= 17.0.0`
* Pipenv `>= 2023.10.24`

These dependencies are automatically installed and managed via Pipenv:

* Conan `>= 2.0.13`

These dependencies are automatically installed and managed via Conan:

* CMake `>= 3.24.2`
* Ninja `>= 1.11.0`

## Packaging

This library can be packaged with Conan, by running:

```
pipenv run conan create -pr:b <YOUR_PROFILE> .
```

## Testing

To run the tests:

```
build/Debug/generators/conanbuild.bat
ctest --preset dev
```

## Development

### Clangd

This project is configured for clangd. Run as follows:

```
clangd --header-insertion=never --compile-commands-dir=build --enable-config
```

### Configuration

Several pre-processor definitions configure behaviour:

|Symbol|Meaning
|-|-
|CCL_HASHTABLE_MINIMUM_CAPACITY|Minimum capacity of a hashtable
|CCL_HASHTABLE_MINIMUM_CHUNK_SIZE|Minimum number of items storable in a hashtable under the same key
|CCL_SET_MINIMUM_CAPACITY|Minimum capacity of a set
|CCL_SET_KEY_CHUNK_SIZE|Max number of consecutive set slots to look for when inserting, before rehashing into a larger set
|CCL_ALLOCATOR_DEFAULT_ALIGNMENT|Default allocator minimum alignment constraint
|CCL_PAGE_SIZE|Page size for paged data structures, as number of elements
|CCL_DEQUE_MIN_CAPACITY|Minimum allocatable capacity for deques
|CCL_ALLOCATOR_IMPL|Enable compiling the default implementations of `ccl::get_default_allocator()` and `ccl::set_default_allocator()`
|CCL_ALLOCATOR_EXPORTER|Mark `ccl::get_default_allocator()` and `ccl::set_default_allocator()` as dll-exported
|CCL_ALLOCATOR_IMPORTER|Mark `ccl::get_default_allocator()` and `ccl::set_default_allocator()` as dll-imported
|CCL_ALLOCATOR_DEFAULT_FLAGS|Set default allocation flags
|CCL_CHAR_TRAITS_DEFAULT_POS_TYPE|The default position type for `char_traits<char>`

Default values are available in [definitions.hpp](include/ccl/definitions.hpp).

Define any of these before including any CCL headers to override the default behaviour. The best
way to override these is by leveraging your compiler flags (i.e. `g++ -DCCL_PAGE_SIZE=1024 ...`). Default values are meant to be reasonable for most situations. Some values or combinations
may not make sense and cause undefined behaviour. Be sure to check the source code first.

Certain features can be enabled or disabled as needed. The following pre-processor definitions configure optional features:

|Symbol|Meaning
|-|-
|CCL_FEATURE_ASSERTIONS|Enable assertions
|CCL_FEATURE_EXCEPTIONS|Enable exceptions
|CCL_FEATURE_TYPECHECK_CASTS|Enable use of `dynamic_cast` where appropriate
|CCL_FEATURE_STL_COMPAT|Include the STL compatibility header
|CCL_FEATURE_DEFAULT_ALLOCATION_FLAGS|Enable default allocation flags. Disabling this flag will result in a compilation error whenever default allocation flags are not manually defined

Default values are available in [features.hpp](include/ccl/features.hpp).

To override the presence of any feature, define `CCL_OVERRIDE_FEATURE_<FEATURE_NAME>`. This will disable the definition any pre-processor symbols related to the given feature.

### STL Compatibility Layer

There is an STL compatibility layer available in [compat.hpp](include/ccl/compat.hpp). This file is not included by default and provides definitions for enhancing compatibility of CCL with existing STL constructs.
