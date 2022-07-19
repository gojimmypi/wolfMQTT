# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# standard debug build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS -DDEBUG_WOLFSSL")

# BEGIN my_private_config
#
# we'll look for a my_private_config.h in various environments
# we also assume that the file is added to the local .gitignore
# to ensure it is never inadvertently shared

if(EXISTS "/c/workspace/my_private_config.h")
   message(STATUS "found SYSPROGS_MY_PRIVATE_CONFIG")
   add_definitions( -DSYSPROGS_MY_PRIVATE_CONFIG="/c/workspace/my_private_config.h" )
endif()

if(EXISTS "/workspace/my_private_config.h")
   message(STATUS "found WINDOWS_MY_PRIVATE_CONFIG")
   add_definitions( -DWINDOWS_MY_PRIVATE_CONFIG="/workspace/my_private_config.h" )
endif()

if(EXISTS "/mnt/c/workspace/my_private_config.h")
   message(STATUS "found WSL_MY_PRIVATE_CONFIG")
   add_definitions( -DWSL_MY_PRIVATE_CONFIG="/mnt/c/workspace/my_private_config.h" )
endif()

if(EXISTS "(~/my_private_config.h")
   message(STATUS "found LINUX_MY_PRIVATE_CONFIG")
   add_definitions( -DWSL_MY_PRIVATE_CONFIG="~/my_private_config.h" )
endif()
# END my_private_config

include($ENV{IDF_PATH}/tools/cmake/project.cmake)


project(wolfMQTT_client_RISCV)
