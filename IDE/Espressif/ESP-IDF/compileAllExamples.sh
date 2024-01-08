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

echo "Found IDF_PATH = $IDF_PATH"
echo ""

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

echo "************************************************************************"
echo "* Completed wolfMQTT compileAllExamples.sh"
echo "************************************************************************"
