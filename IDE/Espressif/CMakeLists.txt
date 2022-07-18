# CMakeLists.txt
#
# Copyright (C) 2006-2022 wolfSSL Inc.
#
# This file is part of wolfMQTT.
#
# wolfMQTT is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# wolfMQTT is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
#
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# standard debug build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS -DDEBUG_WOLFSSL")

# standard no debug build
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")

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

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# copy enc28j60 component. This section can be removed if never using the enc28j60
if(EXISTS "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60")
    message(STATUS  "Found directory ${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60")
else()
    message(STATUS  "Making directory ${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60")
    make_directory( "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60")
endif()


if(EXISTS "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/enc28j60.h")
    message(STATUS  "Found ${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/enc28j60.h")

elseif(EXISTS "$ENV{IDF_PATH}/examples/ethernet/enc28j60/components/eth_enc28j60/")
      message(STATUS  "Copying files  ${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60")
      file(COPY "$ENV{IDF_PATH}/examples/ethernet/enc28j60/components/eth_enc28j60/" DESTINATION "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/" FILES_MATCHING PATTERN  "*")

elseif (EXISTS "$ENV{IDF_PATH}/examples/ethernet/enc28j60/main/")
    # some versions of the ESP-IDF (typically prior to 4.4) don't have the eth_enc28j60 in a component directory,rather just an example
    file(COPY "$ENV{IDF_PATH}/examples/ethernet/enc28j60/main/enc28j60.h"             DESTINATION "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/")
    file(COPY "$ENV{IDF_PATH}/examples/ethernet/enc28j60/main/esp_eth_enc28j60.h"     DESTINATION "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/")
    file(COPY "$ENV{IDF_PATH}/examples/ethernet/enc28j60/main/esp_eth_mac_enc28j60.c" DESTINATION "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/")
    file(COPY "$ENV{IDF_PATH}/examples/ethernet/enc28j60/main/esp_eth_phy_enc28j60.c" DESTINATION "${CMAKE_HOME_DIRECTORY}/components/eth_enc28j60/")
endif()
# END OF enc28j60

project(wolfmqtt_client)

# check to see if there's both a local copy and EDP-IDF copy of the wolfssl and/or wolfMQTT components
if( EXISTS "${CMAKE_HOME_DIRECTORY}/components/wolfssl/" AND EXISTS "$ENV{IDF_PATH}/components/wolfssl/" )
    message(STATUS  "")
    message(STATUS  "WARNING: Found components/wolfssl in both local project and IDF_PATH")
    message(STATUS  "")
endif()

if( EXISTS "${CMAKE_HOME_DIRECTORY}/components/wolfMQTT/" AND EXISTS "$ENV{IDF_PATH}/components/wolfMQTT/" )
    message(STATUS  "")
    message(STATUS  "WARNING: Found components/wolfMQTT in both local project and IDF_PATH")
    message(STATUS  "")
endif()
# end multiple component check

# some checks on wolfSSL and wolfMQTT user settings.
# Note the components could live in either ESP IDF or local project component directory
# for sanity, choose only one location.

if( EXISTS "${CMAKE_HOME_DIRECTORY}/components/wolfssl/include/user_settings.h" )
    message( "" )
    message("wolfSSL user_settings.h found in ${CMAKE_HOME_DIRECTORY}/components/wolfssl/ ")

    if( EXISTS "${CMAKE_HOME_DIRECTORY}/components/wolfMQTT/include/user_settings.h" )
        message( "" )
        message("WARNING: additional wolfMQTT user_settings.h found in local project.")
        message( "" )

        message("user_settings.h found in ${CMAKE_HOME_DIRECTORY}/components/wolfMQTT/ ")

        execute_process( COMMAND ${CMAKE_COMMAND} -E compare_files "${CMAKE_HOME_DIRECTORY}/components/wolfssl/include/user_settings.h"  "${CMAKE_HOME_DIRECTORY}/components/wolfMQTT/include/user_settings.h"       RESULT_VARIABLE wolfMQTT_WOLFSSL_COMPARE )

        if( wolfMQTT_WOLFSSL_COMPARE EQUAL 0)
            message("Confirmed wolfSSL and wolfMQTT user_settings.h match in local project.")
        elseif( wolfMQTT_WOLFSSL_COMPARE EQUAL 1)
            message( "" )
            message("WARNING: wolfSSL and wolfMQTT user_settings.h DO NOT MATCH in local project.")
            message( "" )
        else()
            message("Error while comparing wolfSSL and wolfMQTT user_settings.h files in local project.")
        endif()
    else()
        message( "" )
        message("Confirmed extra wolfMQTT user_settings.h not found in local project.")
        message( "" )
    endif()

endif()

if( EXISTS "$ENV{IDF_PATH}/components/wolfssl/include/user_settings.h" )
    message( "" )
    message("wolfSSL user_settings.h found in $ENV{IDF_PATH}/components/wolfssl/ ")

    if( EXISTS "$ENV{IDF_PATH}/components/wolfMQTT/include/user_settings.h" )
        message( "" )
        message("WARNING: wolfMQTT user_settings.h not found in ESP-IDF.")
        message( "" )

        message("user_settings.h found in $ENV{IDF_PATH}/components/wolfMQTT/ ")

        execute_process( COMMAND ${CMAKE_COMMAND} -E compare_files "$ENV{IDF_PATH}/components/wolfssl/include/user_settings.h"  "$ENV{IDF_PATH}/components/wolfMQTT/include/user_settings.h"       RESULT_VARIABLE wolfMQTT_WOLFSSL_COMPARE )

        if( wolfMQTT_WOLFSSL_COMPARE EQUAL 0)
            message("Confirmed wolfSSL and wolfMQTT user_settings.h match in ESP-IDF.")
        elseif( wolfMQTT_WOLFSSL_COMPARE EQUAL 1)
            message( "" )
            message("WARNING: wolfSSL and wolfMQTT user_settings.h DO NOT MATCH in ESP-IDF.")
            message( "" )
        else()
            message("Error while comparing wolfSSL and wolfMQTT user_settings.h files in ESP-IDF.")
        endif()
    else()
        message( "" )
        message("Confirmed extra wolfMQTT user_settings.h not found in ESP-IDF.")
        message( "" )
    endif()
endif()
# end checks on wolfSSL and wolfMQTT user settings.