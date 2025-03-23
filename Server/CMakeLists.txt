add_library(Server STATIC
        WordsServer.cpp
        WordsServer.h
        ThreadPool.cpp
        ThreadPool.h

)

target_include_directories(Server PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(Server PUBLIC
        Qt6::Core
        Qt6::Network
        Sockets
)
