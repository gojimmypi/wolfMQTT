#!/bin/bash
#
# wolfMQTT testing script: compileAllExamples
#

echo "Starting wolfMQTT compileAllExamples.sh"

if  [[ "$IDF_PATH" == "" ]]; then
    echo "Error: $IDF_PATH not found; run Espressif export.sh"
    exit 1
fi

SCRIPT_DIR=$(builtin cd ${BASH_SOURCE%/*}; pwd)
RUN_SETUP=$1
THIS_ERR=0

echo "Found IDF_PATH = $IDF_PATH"

