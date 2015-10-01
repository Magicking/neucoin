find_path(MINIUPNPC_INCLUDES_DIR
  miniupnpc/miniwget.h
  miniupnpc/miniupnpc.h
  miniupnpc/upnperrors.h
  miniupnpc/upnpcommands.h
  PATHS /usr/local/include /usr/include/
)

find_library(MINIUPNPC_LIBRARY
  NAMES miniupnpc
  PATHS /usr/lib /usr/local/lib
)

if(MINIUPNPC_LIBRARY AND MINIUPNPC_INCLUDES_DIR)
  if(NOT MINIUPNPC_FIND_QUIETLY)
    message(STATUS "Found miniupnpc library at ${MINIUPNPC_LIBRARY}")
    message(STATUS "Found miniupnpc header at ${MINIUPNPC_INCLUDES_DIR}")
  endif()
  set(CMAKE_REQUIRED_INCLUDES  ${MINIUPNPC_INCLUDES_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${MINIUPNPC_LIBRARY})

  include(CheckCXXSourceRuns)

  CHECK_CXX_SOURCE_RUNS("
  #include <cstdlib>
  #include <miniupnpc/miniwget.h>
  #include <miniupnpc/miniupnpc.h>
  #include <miniupnpc/upnperrors.h>
  #include <miniupnpc/upnpcommands.h>
  int main()
  {
  int major = 0, minor = 0;
  char *s;
  #ifdef MINIUPNPC_VERSION
  major = strtol(MINIUPNPC_VERSION, &s, 10);
  minor = (*s && *(s + 1)) ? strtol(s + 1, NULL, 10) : 0;
  #endif
  return !((major > 1) || (major == 1 && minor >= 5));
  }"
  MINIUPNPC_VERSION_1_5_OR_HIGHER)
  if(MINIUPNPC_VERSION_1_5_OR_HIGHER)
    set(MINIUPNPC_VERSION "1.5")
  endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(miniupnpc REQUIRED_VARS MINIUPNPC_INCLUDES_DIR MINIUPNPC_LIBRARY MINIUPNPC_VERSION
                                        VERSION_VAR MINIUPNPC_VERSION)
MARK_AS_ADVANCED(MINIUPNPC_INCLUDES_DIR MINIUPNPC_LIBRARY)
