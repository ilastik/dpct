# This module finds an installed Lemon package.
#
# It sets the following variables:
#  Lemon_FOUND              - Set to false, or undefined, if lemon isn't found.
#  Lemon_INCLUDE_DIR        - Lemon include directory.
#  Lemon_LIBRARIES          - Lemon library files
FIND_PATH(Lemon_INCLUDE_DIR lemon/config.h PATHS /usr/include /usr/local/include ${CMAKE_INCLUDE_PATH} ${CMAKE_PREFIX_PATH}/include $ENV{Lemon_ROOT}/include ENV CPLUS_INCLUDE_PATH)
FIND_LIBRARY(Lemon_LIBRARIES
  NAMES emon lemon
  PATHS $ENV{Lemon_ROOT}/src/impex $ENV{Lemon_ROOT}/lib ENV LD_LIBRARY_PATH ENV LIBRARY_PATH
)

GET_FILENAME_COMPONENT(Lemon_LIBRARY_PATH ${Lemon_LIBRARIES} PATH)
SET( Lemon_LIBRARY_DIR ${Lemon_LIBRARY_PATH} CACHE PATH "Path to lemon library.")

# handle the QUIETLY and REQUIRED arguments and set Lemon_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lemon DEFAULT_MSG Lemon_LIBRARIES Lemon_INCLUDE_DIR)

MARK_AS_ADVANCED( Lemon_INCLUDE_DIR Lemon_LIBRARIES Lemon_LIBRARY_DIR )
