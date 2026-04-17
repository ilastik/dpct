cmake_minimum_required(VERSION 3.27)
message( "\nConfiguring python wrapper:" )

find_package(Python REQUIRED Interpreter Development)
find_package(pybind11 CONFIG)

# include external headers as system includes so we do not have to cope with their warnings
include_directories( SYSTEM ${PYTHON_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} )

set(PYDPCT_SRCS pydpct.cpp pythongraphreader.cpp)
pybind11_add_module(dpct ${PYDPCT_SRCS})
target_link_libraries(dpct PRIVATE dpct_core)
target_include_directories(dpct PRIVATE ${DPCT_CORE_INCLUDE_DIR})

install(TARGETS dpct
          LIBRARY DESTINATION ${Python_SITELIB}
          RUNTIME DESTINATION ${Python_SITELIB})
