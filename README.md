# CCL C++ base library

## API stability

API and implementation stable: 🟢

API stable: 🟡

Unstable: 🔴

|Interface|Status
|-|-
|Utilities|🔴
|Vector|🔴
|Test Driver|🟡
|Maybe|🔴
|Hashtable|🔴
|Hash|🔴
|Debug Utilities|🔴
|Compressed Pair|🔴
|Bitset|🔴
|Allocators|🔴
|Tables|🔴
|ECS|🔴
|Dense Map|🔴
|Typed Handle|🔴
|Versioned Handle|🔴
|Handle Manager|🔴
|Packed Integer|🔴
|Paged Vector|🔴
|Set|🔴
|Sparse Set|🔴
|Tagged pointer|🔴
|Pair|🔴
|Atomic|🔴
|Deque|🔴
|Pool|🔴
|Memory Pool|🔴
|Shared Pointer|🔴

## Building

While this is a header-only library, the build process is required to create the *version.hpp* header
file.

This library is built with cmake `>= 3.24.2`. Lower versions may work but are not supported. The following are example build commands for informational purposes only. The specific commands for your system may differ.

To initialise the build system for development, run:

```
cmake --preset dev
```

Or with `--preset release` to use the release configuration.

To enable test coverage data to be collected, append `-DCCL_COVERAGE:BOOL=ON` - enabled by default for the *dev* preset. This will enable instrumentation of tests in order to collect coverage data, the `coverage-summary` and `coverage-report` build targets.

Similarly, to build the project:

```
cmake --build --preset dev
```

### Build dependencies

* Cmake `>= 3.24.2`
* LLVM `>= 15.0.0`
* Ninja `>= 1.11.0`
* Conan `>= 1.55.0`

## Packaging

This library can be packaged with Conan, by running:

```
conan create -pr:b <YOUR_PROFILE> .
```

## Testing

To run the tests:

```
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
|CCL_HANDLE_VALUE_WIDTH|Handle value width, in bits
|CCL_ECS_VIEW_MAX_ARCHETYPE_COUNT|Maximum number of archetypes iterable from a view
|CCL_HASHTABLE_MINIMUM_CAPACITY|Minimum capacity of a hashtable
|CCL_HASHTABLE_CHUNK_SIZE|Max number of consecutive hashtable slots to look for when inserting, before rehashing into a larger table
|CCL_SET_MINIMUM_CAPACITY|Minimum capacity of a set
|CCL_SET_KEY_CHUNK_SIZE|Max number of consecutive set slots to look for when inserting, before rehashing into a larger set
|CCL_ALLOCATOR_DEFAULT_ALIGNMENT|Default allocator minimum alignment constraint
|CCL_PAGE_SIZE|Page size for paged data structures, as number of elements
|CCL_DEQUE_MIN_CAPACITY|Minimum allocatable capacity for deques
|CCL_ALLOCATOR_IMPL|Enable compiling the default implementations of `ccl::get_default_allocator()` and `ccl::set_default_allocator()`
|CCL_ALLOCATOR_EXPORTER|Mark `ccl::get_default_allocator()` and `ccl::set_default_allocator()` as dll-exported
|CCL_ALLOCATOR_IMPORTER|Mark `ccl::get_default_allocator()` and `ccl::set_default_allocator()` as dll-imported

Default values are availble in [definitions.hpp](include/ccl/definitions.hpp).

Define any of these before including any CCL headers to override the default behaviour. The best
way to override these is by leveraging your compiler flags (i.e. `g++ -DCCL_HANDLE_VALUE_WIDTH=16 ...`). Default values are meant to be reasonable for most situations. Some values or combinations
may not make sense and cause undefined behaviour. Be sure to check the source code first.

Certain features can be enabled or disabled as needed. The following pre-processor definitions configure optional features:

|Symbol|Meaning
|-|-
|CCL_FEATURE_ASSERTIONS|Enable assertions
|CCL_FEATURE_EXCEPTIONS|Enable exceptions
|CCL_FEATURE_TYPECHECK_CASTS|Enable use of `dynamic_cast` where appropriate
|CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS|Control whether the ECS registry checks for existing components before adding/removing new ones. Disabling this feature may yield unexpected results if adding already existing components or removing non-existent ones.
|CCL_FEATURE_ECS_CHECK_UNSAFE_REMOVE_ENTITY|Assert the presence of the entity when calling `unsafe_remove_entity()`.

Default values are availble in [features.hpp](include/ccl/features.hpp).

To override the presence of any feature, define `CCL_OVERRIDE_FEATURE_<FEATURE_NAME>`. This will disable the definition any pre-processor symbols related to the given feature.

### STL Compatibility Layer

There is an STL compatibility layer available in [compat.hpp](include/ccl/compat.hpp). This file is not included by default and provides definitions for enhancing compatibility of CCL with existing STL constructs.
