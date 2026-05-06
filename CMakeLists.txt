cmake_minimum_required(VERSION 3.27)
project(dpct)

include(CMakePackageConfigHelpers)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_extensions/)
# ------------------------------------------
# user defined configuration
option(WITH_TESTS "Build tests." OFF)
option(WITH_LOG "Enable logs." OFF)
option(WITH_PYTHON "Build python wrapper." ON)
option(WITH_BIN "Build track executable." ON)
option(MULTI_STAGE_BUILD "Enable conda build of the Python packages with multiple outputs" OFF)


if(WITH_LOG)
    add_definitions(-DDEBUG_LOG)
endif()

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C++ STL debug symbols
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(WITH_DEBUG_STL "False" CACHE BOOL "Build with C++ stl debug symbols?")
  if(WITH_DEBUG_STL)
    add_definitions(-D_GLIBCXX_DEBUG)
  endif()
endif()


if (MULTI_STAGE_BUILD)
    find_package(dpct REQUIRED)
else()
    find_package(Lemon REQUIRED)
    find_package(jsoncpp REQUIRED)
    file(GLOB_RECURSE SOURCES src/*.cpp)
    file(GLOB_RECURSE HEADERS include/*.h)
    include_directories(${Lemon_INCLUDE_DIR})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
    add_library(dpct SHARED ${SOURCES} ${HEADERS})
    target_link_libraries(dpct ${Lemon_LIBRARIES} JsonCpp::JsonCpp)

    target_include_directories(dpct
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    add_library(dpct::dpct ALIAS dpct)
    set_target_properties(dpct PROPERTIES
        EXPORT_NAME dpct
    )

    # installation
    install(TARGETS dpct
        EXPORT dpctTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION include/dpct
        FILES_MATCHING PATTERN "*.h"
    )

    install(EXPORT dpctTargets
        FILE dpctTargets.cmake
        DESTINATION lib/cmake/dpct
        NAMESPACE dpct::
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/dpctConfigVersion.cmake"
        VERSION 1.5.0
        COMPATIBILITY SameMajorVersion
    )
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dpctConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/dpctConfig.cmake"
        INSTALL_DESTINATION lib/cmake/dpct
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/dpctConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/dpctConfigVersion.cmake"
        DESTINATION lib/cmake/dpct
    )


endif()

# ------------------------------------------
# add subdirectories
if(WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(WITH_PYTHON)
  add_subdirectory(python)
endif()

if(WITH_BIN)
    add_subdirectory(bin)
endif()
