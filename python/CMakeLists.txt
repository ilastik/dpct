cmake_minimum_required(VERSION 3.10)
message( "\nConfiguring python wrapper:" )

# dependencies
find_package( PythonInterp REQUIRED )
# python version needs to be specified since boost 1.67 on windows
find_package( Boost REQUIRED COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} serialization )

find_package( PythonLibs REQUIRED )

# include external headers as system includes so we do not have to cope with their warnings
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS} )

# pydpct
set(PYDPCT_SRCS pydpct.cpp pythongraphreader.cpp)

add_library(pydpct SHARED ${PYDPCT_SRCS})

# building with the "new" conda compilers resulted in strange test errors
# like: TypeError: Expecting an object of type dict; got an object of type dict instead
# Solution is not to link against python during the build
# See: https://github.com/casacore/python-casacore/issues/144
if(APPLE)
  target_link_libraries(pydpct dpct ${Boost_LIBRARIES} )
else()
  target_link_libraries(pydpct dpct ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} )
endif()

if(WIN32)
  set_target_properties(pydpct PROPERTIES PREFIX "" OUTPUT_NAME "dpct" SUFFIX ".pyd")
else()
  set_target_properties(pydpct PROPERTIES PREFIX "" OUTPUT_NAME "dpct" SUFFIX ".so")
endif()

# install

# this will set the install dir to $PYTHON_SITE_PACKAGES
# make use of ${PYTHON_EXECUTABLE} instead of /usr/bin/python
if(NOT DEFINED PYDPCT_INSTALL_DIR OR PYDPCT_INSTALL_DIR MATCHES "^$")
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c 
                    "from distutils.sysconfig import *; print(get_python_lib(1))"
                    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
    file(TO_CMAKE_PATH ${PYTHON_SITE_PACKAGES} PYDPCT_INSTALL_DIR)
endif()

set(PYDPCT_INSTALL_DIR ${PYDPCT_INSTALL_DIR} CACHE PATH "where to install the dpct Python package" FORCE)

# this is the install path relative to CMAKE_INSTALL_PREFIX,
# use this in INSTALL() commands to get packaging right
file(RELATIVE_PATH PYDPCT_INSTALL_DIR ${CMAKE_INSTALL_PREFIX} ${PYDPCT_INSTALL_DIR})

install(TARGETS pydpct 
    RUNTIME DESTINATION ${PYDPCT_INSTALL_DIR}
    LIBRARY DESTINATION ${PYDPCT_INSTALL_DIR})
