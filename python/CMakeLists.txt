cmake_minimum_required(VERSION 3.27)
message( "\nConfiguring python wrapper:" )

find_package(Python REQUIRED Interpreter Development)
find_package(pybind11 CONFIG)

# include external headers as system includes so we do not have to cope with their warnings
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})

set(PYDPCT_SRCS pydpct.cpp pythongraphreader.cpp)
pybind11_add_module(dpct_python ${PYDPCT_SRCS})
set_target_properties(dpct_python PROPERTIES OUTPUT_NAME dpct)
target_link_libraries(dpct_python PRIVATE dpct)
target_include_directories(dpct_python PRIVATE ${DPCT_INCLUDE_DIR})

install(TARGETS dpct_python
          LIBRARY DESTINATION ${Python_SITELIB}
          RUNTIME DESTINATION ${Python_SITELIB})
