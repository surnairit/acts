acts_add_library(
    ExamplesDetectorTelescopeGen3
    SHARED
    src/TelescopeGen3Detector.cpp
    src/TelescopeGen3DetectorElement.cpp
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
