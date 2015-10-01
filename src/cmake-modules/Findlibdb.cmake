find_path(DB_INCLUDE_DIR db_cxx.h
  PATHS /usr/local/include /usr/include/
)

find_library(DB_LIBRARY
  NAMES db_cxx
  PATHS /usr/lib /usr/local/lib
)

if(DB_LIBRARY AND DB_INCLUDE_DIR)
  if(NOT DB_FIND_QUIETLY)
    message(STATUS "Found Berkley DB library at ${DB_LIBRARY}")
    message(STATUS "Found Berkley DB header at ${DB_INCLUDE_DIR}")
  endif()
  set(CMAKE_REQUIRED_INCLUDES  ${DB_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${DB_LIBRARY})
  include(CheckCXXSourceRunsWithOutput)

  CHECK_CXX_SOURCE_RUNS_WITH_OUTPUT("
  #include <iostream>
  #include <db_cxx.h>
  int main()
  {
  int major, minor;
  DbEnv::version(&major, &minor, NULL);
  std::cout << major << \".\" << minor;
  return 0;
  }"
  libdb_version LIBDB_RESULT)
  if(libdb_version)
    set(DB_VERSION "${LIBDB_RESULT}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libdb REQUIRED_VARS DB_INCLUDE_DIR DB_LIBRARY DB_VERSION
                                        VERSION_VAR DB_VERSION)
MARK_AS_ADVANCED(DB_INCLUDE_DIR DB_LIBRARY)
