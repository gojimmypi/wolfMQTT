#
# wolfssl_mqtt_aws_iot/main/CMakeLists.txt
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_USER_SETTINGS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_MAIN_DRIVER")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_AWSIOT_EXAMPLE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_MQTT_TLS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFMQTT_USER_SETTINGS")


set (git_cmd "git")

if( EXISTS "${CMAKE_HOME_DIRECTORY}/components/wolfssl/" AND EXISTS "$ENV{IDF_PATH}/components/wolfssl/" )
    #
    # wolfSSL found in both ESP-IDF and local project - needs to be resolved by user
    #
    message(STATUS "")
    message(STATUS "WARNING: Found components/wolfssl in both local project and IDF_PATH")
    message(STATUS "")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWOLFSSL_MULTI_INSTALL_WARNING")
endif()

if(CMAKE_BUILD_EARLY_EXPANSION)
    set(THIS_RELATIVE_PATH "../../../../../../../..")
    get_filename_component(THIS_ABSOLUTE_PATH  "${THIS_RELATIVE_PATH}/" ABSOLUTE)
    message(STATUS "CMAKE_BUILD_EARLY_EXPANSION idf_component_register THIS_ABSOLUTE_PATH = ${THIS_ABSOLUTE_PATH}")


else()
    set(THIS_RELATIVE_PATH "../../../../../..")
    get_filename_component(THIS_ABSOLUTE_PATH  "${THIS_RELATIVE_PATH}/" ABSOLUTE)
    message(STATUS "idf_component_register THIS_ABSOLUTE_PATH = ${THIS_ABSOLUTE_PATH}")
endif()


    if (EXISTS "${THIS_ABSOLUTE_PATH}/wolfmqtt/mqtt_socket.h")
        get_filename_component(WOLFMQTT_ROOT  "${THIS_RELATIVE_PATH}/" ABSOLUTE)
        set(TESTDIR = "${CMAKE_CURRENT_SOURCE_DIR}")
        message(STATUS "Using wolfMQTT example with root ${WOLFMQTT_ROOT}")
        message(STATUS "TESTDIR =  ${TESTDIR}")
    else()
        get_filename_component(WOLFMQTT_ROOT  "${THIS_RELATIVE_PATH}" ABSOLUTE)
        message(STATUS "Using wolfMQTT example with root ${WOLFMQTT_ROOT}")
        message(ERROR "ERROR: wolfMQTT Source not found")
    endif()
    message(STATUS "main WOLFMQTT_ROOT = ${WOLFMQTT_ROOT}")

## register_component()
idf_component_register(SRCS
                       	    main.c
                       	    ${WOLFMQTT_ROOT}/examples/aws/awsiot.c
                       	    ${WOLFMQTT_ROOT}/examples/mqttexample.c
                       	    ${WOLFMQTT_ROOT}/examples/mqttnet.c
                       	    ${WOLFMQTT_ROOT}/examples/mqttport.c
                       	    wifi_connect.c
                       	    time_helper.c
                        INCLUDE_DIRS
                            "."
                            "./include"
                            ${WOLFMQTT_ROOT}/examples/aws/
                        )

#

#
# LIBWOLFSSL_SAVE_INFO(VAR_OUPUT THIS_VAR VAR_RESULT)
#
# Save the THIS_VAR as a string in a macro called VAR_OUPUT
#
# VAR_OUPUT:  the name of the macro to define
# THIS_VAR:   the OUTPUT_VARIABLE result from a execute_process()
# VAR_RESULT: the RESULT_VARIABLE from a execute_process(); "0" if successful.
#
function ( LIBWOLFSSL_SAVE_INFO VAR_OUPUT THIS_VAR VAR_RESULT )
    # is the RESULT_VARIABLE output value 0? If so, IS_VALID_VALUE is true.
    string(COMPARE EQUAL "${VAR_RESULT}" "0" IS_VALID_VALUE)

    # if we had a successful operation, save the THIS_VAR in VAR_OUPUT
    if(${IS_VALID_VALUE})
        # strip newline chars in THIS_VAR parameter and save in VAR_VALUE
        string(REPLACE "\n" ""  VAR_VALUE  ${THIS_VAR})

        # we'll could percolate the value to the parent for possible later use
        # set(${VAR_OUPUT} ${VAR_VALUE} PARENT_SCOPE)

        # but we're only using it here in this function
        set(${VAR_OUPUT} ${VAR_VALUE})

        # we'll print what we found to the console
        message(STATUS "Found ${VAR_OUPUT}=${VAR_VALUE}")

        # the interesting part is defining the VAR_OUPUT name a value to use in the app
        add_definitions(-D${VAR_OUPUT}=\"${VAR_VALUE}\")
    else()
        # if we get here, check the execute_process command and parameters.
        message(STATUS "LIBWOLFSSL_SAVE_INFO encountered a non-zero VAR_RESULT")
        set(${VAR_OUPUT} "Unknown")
    endif()
endfunction() # LIBWOLFSSL_SAVE_INFO

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    # LIBWOLFSSL_VERSION_GIT_HASH
    execute_process(COMMAND ${git_cmd} "rev-parse" "HEAD" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES ERROR_QUIET )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_HASH "${TMP_OUT}" "${TMP_RES}")

    # LIBWOLFSSL_VERSION_GIT_SHORT_HASH
    execute_process(COMMAND ${git_cmd} "rev-parse" "--short" "HEAD" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES ERROR_QUIET )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_SHORT_HASH "${TMP_OUT}" "${TMP_RES}")

    # LIBWOLFSSL_VERSION_GIT_HASH_DATE
    execute_process(COMMAND ${git_cmd} "show" "--no-patch" "--no-notes" "--pretty=\'\%cd\'" OUTPUT_VARIABLE TMP_OUT RESULT_VARIABLE TMP_RES  )
    LIBWOLFSSL_SAVE_INFO(LIBWOLFSSL_VERSION_GIT_HASH_DATE "${TMP_OUT}" "${TMP_RES}")
endif()

message(STATUS "")

