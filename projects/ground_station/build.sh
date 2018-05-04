#!/bin/bash

# Top level build script for building/uploading
# applications for the ground station controller.

ARDUINO_INSTALL_PATH="$HOME/arduino-1.8.5"
BUILD_ACTION="--upload"
PROG_TARGET=""
BOARD_TARGET="arduino:avr:pro:cpu=8MHzatmega328"
PORT="/dev/ttyUSB0"
WARN_LEVEL="all"
DEBUG_LEVEL=0

CFLAGS=""
CXXFLAGS=""

function usage() {

    echo -e "Usage: build.sh [-h] | [[-vnd] [-b board_target] [-p port_file] [-a arduino_base_dir] file.cpp]"
    echo -e "\t-h\tDisplay this help/usage."
    echo -e "\t-v\tVerify (compile) only, don't upload."
    echo -e "\t-n\tCompile without warnings."
    echo -e "\t-d\tCompile with debug flags."
    echo -e "\t-b\tExamples for board_target are arduino:avr:uno or arduino:avr:pro:cpu=8MHzatmega328"
    echo -e "\t\tSee $ARDUINO_INSTALL_PATH/hardware/arduino/avr/boards.txt for complete list."
    echo -e "\t-p\tCharacter device file which connects to your board. Eg. /dev/ttyACM0"
    echo -e "\t-a\tPath to the Arduino installation directory."
}

if [[ $# -eq 0 ]]; then
    usage
    exit 1
fi

OPTSPEC=":hvndb:p:a:"
while getopts "$OPTSPEC" optchar; do
    case "${optchar}" in
        h)
            usage
            exit 0
            ;;
        v)
            BUILD_ACTION="--verify"
            ;;
        n)
            WARN_LEVEL="none"
            ;;
        d)
            DEBUG_LEVEL=1
            ;;
        b)
            BOARD_TARGET=$OPTARG
            ;;
        p)
            PORT=$OPTARG
            ;;
        a)
            ARDUINO_INSTALL_PATH=$OPTARG
            ;;
        *)
            if [ "$OPTERR" != 1 ] || [ "${OPTSPEC:0:1}" = ":" ]; then
                echo "Non-option argument: '-${OPTARG}'" >&2
            fi
            usage
            exit 1
            ;;
    esac
done

# Check for valid Arduino folder
if [[ ! -d $ARDUINO_INSTALL_PATH ]]; then
    echo "ERROR: Arduino dir \"$ARDUINO_INSTALL_PATH\" does not exist."
    exit 1
fi

# Get last file argument
shift $(($OPTIND - 1))
PROG_TARGET=$1
if [[ ! -f $PROG_TARGET ]]; then
    echo "ERROR: $1 file does not exist"
    exit 1
fi

PROG_TARGET=$(realpath $PROG_TARGET)

# Change directory to Arduino install path
pushd $ARDUINO_INSTALL_PATH > /dev/null
if [[ ! -x ./arduino ]]; then
    echo "ERROR: arduino executable missing or has improper permissions"
fi

# Set debug flag
if [[ $DEBUG_LEVEL -eq 1 ]]; then
    CFLAGS="$CFLAGS -DGS_DEBUG"
    CXXFLAGS="$CXXFLAGS -DGS_DEBUG"
fi

BUILD_CMD="./arduino $BUILD_ACTION $PROG_TARGET \
           --board $BOARD_TARGET --pref compiler.warning_level=$WARN_LEVEL \
           --pref serial.port=$PORT --pref serial.port.file=$PORT \
           --pref compiler.c.extra_flags=\"$CFLAGS\" \
           --pref compiler.cpp.extra_flags=\"$CXXFLAGS\""

echo "Issuing build command:"
echo $BUILD_CMD

# Execute build command
eval $BUILD_CMD

popd > /dev/null
