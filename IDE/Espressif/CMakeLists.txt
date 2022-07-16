# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

#standard build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS -DDEBUG_WOLFSSL")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wolfmqtt_client)
