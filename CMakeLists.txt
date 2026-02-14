# Text2Image Library CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(text2image VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(TEXT2IMAGE_BUILD_TESTS "Build tests" ON)
option(TEXT2IMAGE_BUILD_EXAMPLES "Build examples" ON)
option(TEXT2IMAGE_BUILD_NODEJS "Build Node.js bindings" ON)
option(TEXT2IMAGE_ENABLE_PROFILING "Enable profiling" OFF)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Find Skia
pkg_check_modules(SKIA REQUIRED skia)

# Find libxml2
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

# Find FreeType
pkg_check_modules(FREETYPE REQUIRED freetype2)

# Find libvips
pkg_check_modules(LIBVIPS REQUIRED vips)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

# Create the library
add_library(text2image SHARED ${SOURCES})

# Include directories
target_include_directories(text2image
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${SKIA_INCLUDE_DIRS}
        ${LIBXML2_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        ${LIBVIPS_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(text2image
    PRIVATE
        ${SKIA_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${LIBVIPS_LIBRARIES}
)

# Add compiler definitions
target_compile_definitions(text2image
    PRIVATE
        TEXT2IMAGE_EXPORTS
        $<$<CONFIG:Debug>:TEXT2IMAGE_DEBUG>
        $<$<BOOL:${TEXT2IMAGE_ENABLE_PROFILING}>:TEXT2IMAGE_PROFILING>
)

# Set library properties
set_target_properties(text2image PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Install targets
install(TARGETS text2image
    EXPORT text2imageTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY include/ DESTINATION include)

# Install CMake package configuration
install(EXPORT text2imageTargets
    FILE text2imageTargets.cmake
    NAMESPACE Text2Image::
    DESTINATION lib/cmake/text2image
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/text2imageConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/text2imageConfig.cmake
    INSTALL_DESTINATION lib/cmake/text2image
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/text2imageConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/text2imageConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/text2imageConfigVersion.cmake
    DESTINATION lib/cmake/text2image
)

# Build tests
if(TEXT2IMAGE_BUILD_TESTS)
    add_subdirectory(test)
endif()

# Build examples
if(TEXT2IMAGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Build Node.js bindings
if(TEXT2IMAGE_BUILD_NODEJS)
    add_subdirectory(nodejs)
endif()