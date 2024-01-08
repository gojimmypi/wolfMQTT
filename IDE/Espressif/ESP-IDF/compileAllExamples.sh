#!/bin/bash
#
# wolfMQTT testing script: compileAllExamples
#

echo "************************************************************************"
echo "* Starting wolfMQTT compileAllExamples.sh"
echo "************************************************************************"

if  [[ "$IDF_PATH" == "" ]]; then
    echo "Error: $IDF_PATH not found; run Espressif export.sh"
    exit 1
fi

SCRIPT_DIR=$(builtin cd ${BASH_SOURCE%/*}; pwd)
RUN_SETUP=$1
THIS_ERR=0

echo "Found IDF_PATH = $IDF_PATH"

echo "************************************************************************"
echo "* template create-project-from-example"
echo "************************************************************************"

idf.py create-project-from-example "wolfssl/wolfssl^5.6.6-stable-update2-esp32:template"

cd template

idf.py build

