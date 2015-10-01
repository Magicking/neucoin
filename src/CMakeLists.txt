cmake_minimum_required(VERSION 2.8.0)
project(neucoin C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")
if(TEST)
  enable_testing()
  find_package(Boost REQUIRED COMPONENTS unit_test_framework)
  add_definitions("-DBOOST_TEST_DYN_LINK")
endif()
find_package(Boost REQUIRED COMPONENTS system filesystem program_options thread)
if(DISABLE_LIBDB_CHECK)
  find_package(libdb REQUIRED)
else()
  find_package(libdb 5.3 EXACT REQUIRED)
endif()
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

option(ENABLE_IPV6 "build with IPv6 support" ON)

set(BOOST_LIBS "-lboost_system -lboost_filesystem -lboost_program_options -lboost_thread")
set(BASE_LIBS "-ldb_cxx -lssl -lcrypto ${BOOST_LIBS}")
set(NEUCOIND_LIBS "-lpthread")

if(ENABLE_IPV6)
  include(CheckStructHasMember)
  check_struct_has_member("struct sockaddr_in6" sin6_addr "netinet/in.h"
                          HAVE_IN6_ADDR)
  if(NOT HAVE_IN6_ADDR)
    message(WARNING "struct sockaddr_in6 not available from netinet/in.h, disabling IPv6 support")
    set(ENABLE_IPV6 OFF
        CACHE BOOL "IPv6 support" FORCE)
  else()
    add_definitions("-DUSE_IPV6")
  endif()
endif()

option(ENABLE_UPNP "build with UPnP support" ON)
if(ENABLE_UPNP)
  find_package(miniupnpc 1.5)
  if(NOT MINIUPNPC_FOUND)
    set(ENABLE_UPNP OFF
        CACHE BOOL "UPnP support" FORCE)
  else()
    add_definitions("-DUSE_UPNP")
    set(NEUCOIND_LIBS "${NEUCOIND_LIBS} -lminiupnpc")
  endif()
endif()

set(NEUCOIND_LIBS ${NEUCOIND_LIBS} ${BASE_LIBS})
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_TEST_DIR "${SOURCE_DIR}/test")

include_directories(
      ${SOURCE_DIR}
      ${SOURCE_DIR}/json)

# {{{ SRCS
set(NEUCOIND_SRCS
      ${SOURCE_DIR}/constants.cpp
      ${SOURCE_DIR}/addrman.cpp
      ${SOURCE_DIR}/bitcoinrpc.cpp
      ${SOURCE_DIR}/CheckClientSanity.cpp
      ${SOURCE_DIR}/checkpoints.cpp
      ${SOURCE_DIR}/crypter.cpp
      ${SOURCE_DIR}/db.cpp
      ${SOURCE_DIR}/GetNextTargetRequired.cpp
      ${SOURCE_DIR}/GetProofOfStakeReward.cpp
      ${SOURCE_DIR}/GetProofOfWorkReward.cpp
      ${SOURCE_DIR}/init.cpp
      ${SOURCE_DIR}/irc.cpp
      ${SOURCE_DIR}/IsValidAmount.cpp
      ${SOURCE_DIR}/kernel.cpp
      ${SOURCE_DIR}/key.cpp
      ${SOURCE_DIR}/keystore.cpp
      ${SOURCE_DIR}/main.cpp
      ${SOURCE_DIR}/netbase.cpp
      ${SOURCE_DIR}/net.cpp
      ${SOURCE_DIR}/noui.cpp
      ${SOURCE_DIR}/protocol.cpp
      ${SOURCE_DIR}/rpcdump.cpp
      ${SOURCE_DIR}/script.cpp
      ${SOURCE_DIR}/scrypt-generic.cpp
      ${SOURCE_DIR}/testnet.cpp
      ${SOURCE_DIR}/util.cpp
      ${SOURCE_DIR}/version.cpp
      ${SOURCE_DIR}/wallet.cpp
      ${SOURCE_DIR}/walletdb.cpp
      ${SOURCE_DIR}/json/json_spirit_reader.cpp
      ${SOURCE_DIR}/json/json_spirit_value.cpp
      ${SOURCE_DIR}/json/json_spirit_writer.cpp)

if(TEST)
  set(NEUCOIN_TESTS_SRCS
      ${SOURCE_TEST_DIR}/base58_tests.cpp
      ${SOURCE_TEST_DIR}/base64_tests.cpp
#      ${SOURCE_TEST_DIR}/Checkpoints_tests.cpp
      ${SOURCE_TEST_DIR}/DoS_tests.cpp
      ${SOURCE_TEST_DIR}/getarg_tests.cpp
      ${SOURCE_TEST_DIR}/key_tests.cpp
      ${SOURCE_TEST_DIR}/miner_tests.cpp
      ${SOURCE_TEST_DIR}/mruset_tests.cpp
      ${SOURCE_TEST_DIR}/multisig_tests.cpp
#      ${SOURCE_TEST_DIR}/rpc_tests.cpp
      ${SOURCE_TEST_DIR}/script_P2SH_tests.cpp
      ${SOURCE_TEST_DIR}/script_tests.cpp
      ${SOURCE_TEST_DIR}/sigopcount_tests.cpp
      ${SOURCE_TEST_DIR}/test_bitcoin.cpp
#      ${SOURCE_TEST_DIR}/transaction_tests.cpp
      ${SOURCE_TEST_DIR}/uint160_tests.cpp
      ${SOURCE_TEST_DIR}/uint256_tests.cpp
      ${SOURCE_TEST_DIR}/util_tests.cpp
      ${SOURCE_TEST_DIR}/wallet_tests.cpp)
# }}}

  add_definitions("-g -O0")
  set(NEUCOIN_TESTS_SRCS_FINAL ${NEUCOIN_TESTS_SRCS} ${NEUCOIND_SRCS})
  list(REMOVE_ITEM NEUCOIN_TESTS_SRCS_FINAL "${SOURCE_DIR}/init.cpp")
  add_executable(neucoin_test ${NEUCOIN_TESTS_SRCS_FINAL})
  set_target_properties(neucoin_test PROPERTIES
              RUNTIME_OUTPUT_DIRECTORY ${SOURCE_DIR})
  target_link_libraries(neucoin_test ${NEUCOIND_LIBS}
                                     "-lboost_unit_test_framework")
  add_test(NAME neucoin_test
           COMMAND "${SOURCE_DIR}/neucoin_test"
           WORKING_DIRECTORY ${SOURCE_DIR})
endif()

add_executable(neucoind ${NEUCOIND_SRCS})
target_link_libraries(neucoind ${NEUCOIND_LIBS})

# vim: filetype=cmake:foldmethod=marker
