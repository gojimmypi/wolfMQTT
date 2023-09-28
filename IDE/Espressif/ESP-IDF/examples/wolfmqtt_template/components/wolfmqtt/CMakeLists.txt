#
# Copyright (C) 2006-2023 wolfSSL Inc.  All rights reserved.
#
# This file is part of wolfMQTT.
#
# Contact licensing@wolfssl.com with any questions or comments.
#
# https://www.wolfssl.com
#/

#
# This is the CMakeLists.txt for the Espressif ESP-IDF wolfMQTT component
#
cmake_minimum_required(VERSION 3.5)

# COMPONENT_NAME = wolfmqtt
# The component name is the directory name. "No feature to change this".
# See https://github.com/espressif/esp-idf/issues/8978#issuecomment-1129892685

# set the root of wolfSSL in top-level project CMakelists.txt:
#   set(WOLFMQTT_ROOT  "C:/some path/with/spaces")
#   set(WOLFMQTT_ROOT  "c:/workspace/wolfmqtt-[username]")
#   set(WOLFMQTT_ROOT  "/mnt/c/some path/with/spaces")
#   or use this logic to assign value from Environment Variable WOLFMQTT_ROOT,
#   or assume this is an example 7 subdirectories below:

# We are typically in [root]/IDE/Espressif/ESP-IDF/examples/wolfssl_test/components/wolfssl
# The root of wolfSSL is 7 directories up from here:
if(CMAKE_BUILD_EARLY_EXPANSION)
    set(THIS_RELATIVE_PATH "../../../../../../..")
    get_filename_component(THIS_ABSOLUTE_PATH  "${THIS_RELATIVE_PATH}/" ABSOLUTE)
    message(STATUS "CMAKE_BUILD_EARLY_EXPANSION idf_component_register THIS_ABSOLUTE_PATH = ${THIS_ABSOLUTE_PATH}")


else()
    set(THIS_RELATIVE_PATH "../../../../../../..")
    get_filename_component(THIS_ABSOLUTE_PATH  "${THIS_RELATIVE_PATH}/" ABSOLUTE)
    message(STATUS "idf_component_register THIS_ABSOLUTE_PATH = ${THIS_ABSOLUTE_PATH}")
endif()

if( "$ENV{IDF_COMPONENT_REGISTRY_URL}" STREQUAL "https://components-staging.espressif.com" )
    if( EXISTS "./managed_components/")
        set(WOLFSSL_COMPONENT_NAME "mywolfssl")
        set(WOLFMQTT_COMPONENT_NAME "mywolfmqtt")
    else()
        set(WOLFSSL_COMPONENT_NAME "wolfssl")
        set(WOLFMQTT_COMPONENT_NAME "wolfmqtt")
    endif()
else()
    set(WOLFSSL_COMPONENT_NAME "wolfssl")
    set(WOLFSSL_COMPONENT_NAME "wolfmqtt")
endif()


# These flags apply only to compiling this component independently.
# be sure to include this like in the root CMakeFiles.txt
# for proper application-wide use of WOLFSSL_USER_SETTINGS:
#
#   add_compile_definitions("WOLFSSL_USER_SETTINGS")
#   add_compile_definitions("WOLFMQTT_USER_SETTINGS")
#
# if using ESP-IDF prior to 4.4, see:
# https://github.com/wolfSSL/wolfssl/issues/5727
if (CONFIG_DEBUG_WOLFMQTT)
    message(STATUS "Config DEBUG_WOLFMQTT is enabled for components/wolfmqtt")
endif()

if (WOLFSSL_USER_SETTINGS)
    message(STATUS "Component wolfmqtt: Found predefined WOLFSSL_USER_SETTINGS")
else()
    # wolfSSL settings should have occurred in project makefiles
    #message(WARNING "Component wolfmqtt: Setting WOLFSSL_USER_SETTINGS \
                     #(should have been a project-wide setting)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWOLFSSL_USER_SETTINGS")
endif()

if (WOLFMQTT_USER_SETTINGS)
    message(STATUS "Component wolfmqtt: Found predefined WOLFMQTT_USER_SETTINGS")
else()
    # wolfSSL settings should have occurred in project makefiles. See
    message(WARNING "Component wolfmqtt: Setting WOLFMQTT_USER_SETTINGS
                     (should have been a project-wide setting)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFMQTT_USER_SETTINGS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWOLFMQTT_USER_SETTINGS")
endif()

# Pick up the KConfig value and assign to cmake variable as appropriate
if(CONFIG_USE_WOLFMQTT_TLS)
    if (CONFIG_DEBUG_WOLFMQTT)
        message(STATUS "Config USE_WOLFMQTT_TLS is enabled for components/wolfmqtt")
    endif()
    set(MQTT_TLS_DEFINE "ENABLE_MQTT_TLS")
    add_compile_definitions("${MQTT_TLS_DEFINE}")
else()
    if (CONFIG_DEBUG_WOLFMQTT)
        message(STATUS "Config USE_WOLFMQTT_TLS is NOT enabled for components/wolfmqtt")
    endif()
    set(MQTT_TLS_DEFINE "")
endif()


if(CMAKE_BUILD_EARLY_EXPANSION)
    message(STATUS "wolfssl component CMAKE_BUILD_EARLY_EXPANSION:")
    idf_component_register(
                            REQUIRES "wolfssl" # "${COMPONENT_REQUIRES}"
                           )
else()
    if (EXISTS "${THIS_ABSOLUTE_PATH}/wolfmqtt/mqtt_socket.h")
        get_filename_component(WOLFMQTT_ROOT  "${THIS_ABSOLUTE_PATH}/" ABSOLUTE)
        set(TESTDIR = "${CMAKE_CURRENT_SOURCE_DIR}")
        message(STATUS "Using wolfMQTT example with root ${WOLFMQTT_ROOT}")
        message(STATUS "TESTDIR =  ${TESTDIR}")
    else()
        get_filename_component(WOLFMQTT_ROOT  "${THIS_ABSOLUTE_PATH}" ABSOLUTE)
        message(STATUS "Using wolfMQTT example with root ${WOLFMQTT_ROOT}")
        message(STATUS "ERROR: component wolfMQTT Source not found")
    endif()

    message(STATUS "wolfmqtt CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")

    set(CMAKE_CURRENT_SOURCE_DIR ".")
#    set(WOLFSSL_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
    set(INCLUDE_PATH ${WOLFMQTT_ROOT})

    set(COMPONENT_SRCDIRS "${WOLFMQTT_ROOT}/src")

    message(STATUS "COMPONENT_SRCDIRS = ${COMPONENT_SRCDIRS}")


    set(COMPONENT_REQUIRES lwip ${WOLFSSL_COMPONENT_NAME})

    set(COMPONENT_ADD_INCLUDEDIRS
        "."
        "./include"
        "${WOLFMQTT_ROOT}"
        "${WOLFMQTT_ROOT}/examples"
        "${WOLFMQTT_ROOT}/wolfmqtt"
        )


    # list(REMOVE_ITEM ASM_FILES  ".*\\.(asm|S)")

    message(STATUS "WOLFSSL_ROOT = ${WOLFSSL_ROOT}")
    message(STATUS "ASM_FILES = ${ASM_FILES}")


    #idf_component_register(
                            #SRC_DIRS "${COMPONENT_SRCDIRS}"
                            #INCLUDE_DIRS "${COMPONENT_ADD_INCLUDEDIRS}"
                            #REQUIRES "${COMPONENT_REQUIRES}"
                            #EXCLUDE_SRCS "${COMPONENT_SRCEXCLUDE}"
                            ## SRCS wolfssl/wolfcrypt/test/test.c
                            #)
        idf_component_register(
                                SRC_DIRS "${COMPONENT_SRCDIRS}"
                                INCLUDE_DIRS "${COMPONENT_ADD_INCLUDEDIRS}"
                                REQUIRES "${COMPONENT_REQUIRES}"
                                EXCLUDE_SRCS "${COMPONENT_SRCEXCLUDE}"
                                PRIV_REQUIRES "${WOLFSSL_COMPONENT_NAME}" # this will typically only be needed for wolfSSL benchmark
                               )
endif()




# check to see if there's both a local copy and EDP-IDF copy of the wolfmqtt components
if( EXISTS "${CMAKE_HOME_DIRECTORY}/components/wolfmqtt/" AND EXISTS "$ENV{IDF_PATH}/components/wolfmqtt/" )
    message(STATUS  "")
    message(WARNING "WARNING: Found components/wolfmqtt in both local project and IDF_PATH")
    message(STATUS  "")
endif()
# end multiple component check

