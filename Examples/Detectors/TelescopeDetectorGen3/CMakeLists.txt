acts_add_library(
    ExamplesDetectorTelescopeGen3
    SHARED
    src/TelescopeDetectorGen3.cpp
    src/TelescopeDetectorGen3Element.cpp 
    src/TelescopeBuilderGen3.cpp
    src/TelescopeBlueprintGen3.cpp
)

acts_compile_headers(ExamplesDetectorTelescopeGen3 GLOB "include/**/*.hpp")

target_include_directories(
    ActsExamplesDetectorTelescopeGen3
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
    ActsExamplesDetectorTelescopeGen3
    PUBLIC Acts::ExamplesDetectorsCommon
)
