include_guard()

option(CCL_FEATURE_BUILD_SHARED "If enabled builds a shared library" OFF)
option(CCL_FEATURE_ASSERTIONS "Enable assertions" ON)
option(CCL_FEATURE_EXCEPTIONS "Enable exceptions" ON)
option(CCL_FEATURE_SANITIZE_MEMORY "Instrument the application to check for uninitialized memory reads" OFF)
option(CCL_FEATURE_SANITIZE_ADDRESS "Instrument the application to check for memory errors" OFF)
option(CCL_FEATURE_SANITIZE_UNDEFINED_BEHAVIOR "Instrument the application to check for undefined behaviour" OFF)
option(CCL_FEATURE_SANITIZE_STACK "Instrument the application to check for stack errors" OFF)
