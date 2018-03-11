#!/bin/bash

# This script takes a text file name as input,
# converts it into a binary form, and then sends
# the output over the serial dev device.
# This is a write only operation

DEFAULT_DEV_DEVICE="/dev/ttyACM0"
DEV_DEVICE=$DEFAULT_DEV_DEVICE

if [[ $# -eq 0 ]]; then
    echo "Usage: $0 ascii_cmd_file [dst_char_dev_file]"
    exit -1
fi

if [[ $# -eq 2 ]]; then
    if [[ -c $2 ]]; then
        DEV_DEVICE=$2
    else
        echo "Error: second arg doesn't exist or is not a char device."
    fi
fi

if [[ ! -f "$1" ]]; then
    echo "Error: $1 file doesn't exist."
    exit -1
fi

cat $1 | xxd -r -p > $DEFAULT_DEV_DEVICE

