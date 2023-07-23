set(LIB_SRC
    async_logging.cpp
    count_down_latch.cpp
    file_util.cpp
    log_file.cpp
    logging.cpp
    log_stream.cpp
    thread.cpp
)


add_library(libserver_base ${LIB_SRC})
target_link_libraries(libserver_base pthread rt)

set_target_properties(libserver_base PROPERTIES OUTPUT_NAME "server_base")

add_subdirectory(tests)