### Cap'n Proto ###

if(BUILD_CLIENT OR BUILD_SERVER)
  lib_buildgen(
    LIBRARY capnproto
    PARAMS "-DCMAKE_DEBUG_POSTFIX=d"
           "-DBUILD_TESTING=OFF")
  lib_build(LIBRARY capnproto)
endif()

# CAPNP_GENERATE_CPP ===========================================================
#
# Example usage:
#   find_package(CapnProto)
#   capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS schema.capnp)
#   add_executable(foo main.cpp ${CAPNP_SRCS})
#   target_link_libraries(foo PRIVATE CapnProto::capnp-rpc)
#   target_include_directories(foo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#
#  If you are not using the RPC features you can use 'CapnProto::capnp' in the
#  target_link_libraries call
#
# Configuration variables (optional):
#   CAPNPC_OUTPUT_DIR
#       Directory to place compiled schema sources (default: CMAKE_CURRENT_BINARY_DIR).
#   CAPNPC_IMPORT_DIRS
#       List of additional include directories for the schema compiler.
#       (CAPNPC_SRC_PREFIX and CAPNP_INCLUDE_DIRECTORY are always included.)
#   CAPNPC_SRC_PREFIX
#       Schema file source prefix (default: CMAKE_CURRENT_SOURCE_DIR).
#   CAPNPC_FLAGS
#       Additional flags to pass to the schema compiler.
#
# TODO: convert to cmake_parse_arguments