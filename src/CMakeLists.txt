set(SRCS
    channel.cpp
    epoll.cpp
    event_loop.cpp
    event_loop_thread.cpp
    event_loop_thread_pool.cpp
    http_data.cpp
    main.cpp
    server.cpp
    thread_pool.cpp
    timer.cpp
    util.cpp
)


add_executable(WebServer ${SRCS})
target_link_libraries(WebServer libserver_base)

add_subdirectory(base)
add_subdirectory(tests)