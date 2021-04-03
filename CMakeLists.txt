cmake_minimum_required(VERSION 2.8.9)
project (fs_websocket_signal)

set ( PROJECT_LINK_LIBS libliquid.so )

add_executable(wssignal wssignal.c)
target_link_libraries(wssignal ${PROJECT_LINK_LIBS} )