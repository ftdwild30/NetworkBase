

include_directories(${ROOT_DIR}/src/base/transport
        ${ROOT_DIR}/src/utils/system)

foreach(target
        test_log)
    add_executable(test_${target} ${target}.cpp)
    target_link_libraries(test_${target} network_static)
endforeach()