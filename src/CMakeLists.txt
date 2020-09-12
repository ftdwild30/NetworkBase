file(GLOB_RECURSE BASE_FILES
        ${ROOT_DIR}/src/base/*.cpp)
file(GLOB_RECURSE UTILS_FILES
        ${ROOT_DIR}/src/utils/*.cpp)

include_directories(${ROOT_DIR}/src/base/transport
        ${ROOT_DIR}/src/base/engine
        ${ROOT_DIR}/src/utils/system
        ${ROOT_DIR}/src/utils/tool)
include_directories(${ROOT_DIR}/build/release/mbedtls/include)

set(ALL_FILES ${BASE_FILES} ${UTILS_FILES})

add_library(network_static STATIC ${ALL_FILES})
set_target_properties(network_static PROPERTIES OUTPUT_NAME "network")
set_target_properties(network_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)