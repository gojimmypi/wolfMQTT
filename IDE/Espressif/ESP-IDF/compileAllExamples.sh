#!/bin/bash
#
# wolfMQTT testing script: compileAllExamples
#

echo "************************************************************************"
echo "************************************************************************"
echo "* Starting wolfMQTT compileAllExamples.sh"
echo "************************************************************************"
echo "************************************************************************"

if  [[ "$IDF_PATH" == "" ]]; then
    echo "Error: $IDF_PATH not found; run Espressif export.sh"
    exit 1
fi

SCRIPT_DIR=$(builtin cd ${BASH_SOURCE%/*}; pwd)
RUN_SETUP=$1
THIS_ERR=0
BUILD_PUBLISHED_EXAMPLES=0

echo "Found IDF_PATH = $IDF_PATH"
echo ""

# Optionally build published examples. (see above; set BUILD_PUBLISHED_EXAMPLES=1)
# The published examples don't change with GitHub, so no need to test every commit.
#
# See also [scripts]/espressif/wolfssl_component_publish.sh
#
    echo "************************************************************************"
    echo "* template create-project-from-example"
    echo "************************************************************************"

    # See https://components.espressif.com/components/wolfssl/wolfmqtt
    idf.py create-project-from-example "wolfssl/wolfssl^5.6.6-stable-update2-esp32:template"

    THIS_ERROR_CODE=$?
    if [ $THIS_ERROR_CODE -ne 0 ]; then
        echo ""
        echo "ERROR: Failed idf.py create-project-from-example for template example"
        exit 1
    fi

    cd template

    idf.py build

    THIS_ERROR_CODE=$?
    if [ $THIS_ERROR_CODE -ne 0 ]; then
        echo ""
        echo "ERROR: Failed to build template from the ESP Registry"
        exit 1
    else
        echo ""
        echo "Successfully built template example from the ESP Registry"
    fi


    echo "************************************************************************"
    echo "* AWS_IoT_MQTT create-project-from-example"
    echo "************************************************************************"

    # See https://components.espressif.com/components/wolfssl/wolfmqtt
    idf.py create-project-from-example "wolfssl/wolfmqtt^1.18.0-preview6:AWS_IoT_MQTT"

    THIS_ERROR_CODE=$?
    if [ $THIS_ERROR_CODE -ne 0 ]; then
        echo ""
        echo "ERROR: Failed idf.py create-project-from-example for AWS_IoT_MQTT example"
        exit 1
    fi

    cd AWS_IoT_MQTT

    idf.py build

    THIS_ERROR_CODE=$?
    if [ $THIS_ERROR_CODE -ne 0 ]; then
        echo ""
        echo "ERROR: Failed to build AWS_IoT_MQTT from the ESP Registry"
        exit 1
    else
        echo ""
        echo "Successfully built AWS_IoT_MQTT from the ESP Registry"
    fi
fi

# NOTE: The wolfMQTT examples include a CMakeLists.txt that will look for wolfssl
# source code. Using managed components will cause a conflict.
# The manifested error will be something like:
#   CMake Error at /opt/esp/idf/tools/cmake/component.cmake:250 (message):
#     ERROR: Cannot process component requirements.  Multiple candidates to
#       satisfy project requirements:
#     requirement: "wolfssl" candidates: "wolfssl, wolfssl__wolfssl"if [ $BUILD_PUBLISHED_EXAMPLES -ne 0 ]; then
# See `components/wolfssl.bak` rename, below, to avoid this error.
# A future CMakeLists.txt may handle this more gracefully.
target=esp32
file=wolfmqtt_template
echo "Building target = ${target} for ${file}"
pushd ${SCRIPT_DIR}/examples/${file}/                                    && \
      rm -rf ./build                                                     && \
      mv components/wolfssl components/wolfssl.bak                       && \
      idf.py add-dependency "wolfssl/wolfssl^5.6.6-stable-update2-esp32" && \
      idf.py set-target ${target} fullclean build
      THIS_ERR=$?
popd
if [ $THIS_ERR -ne 0 ]; then
    echo "Failed target ${target} in ${file}"
    all_build_success=false
fi

echo "All builds successful: ${all_build_success}"
if [[ "${all_build_success}" == "false" ]]; then
  echo "One or more builds failed"
  exit 1
fi

echo "************************************************************************"
echo "* Completed wolfMQTT compileAllExamples.sh"
echo "************************************************************************"

