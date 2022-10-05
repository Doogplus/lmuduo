cmake_minimum_required(VERSION 2.8)
project(lmuduo)

set(CMAKE_CXX_STANDARD 11)
set(WORKSPACE_DIR /home/$ENV{USER}/Code/lmuduo)

include_directories(../)
link_directories("${WORKSPACE_DIR}/lib/")
link_libraries(
        lmuduo
        pthread
)

##################################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./bin")
add_executable(time_test time_test.cpp)
add_executable(inet_addr_test inet_addr_test.cpp ../TcpServer.h ../EventLoop.h ../EventLoop.cpp ../Channel.h ../Channel.cpp)
add_executable(tcp_server_test tcp_server_test.cpp)
