#!/usr/bin/env bash

BLB_DEFAULT_BUILD_DIR="src"

BLB_ORIGINAL_CWD=`pwd`
BLB_SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BLB_BUILD_DIR="${BLB_BUILD_DIR-$BLB_SCRIPT_DIR/$BLB_DEFAULT_BUILD_DIR}"
BLB_OUT_DIR="${BLB_OUT_DIR-$BLB_SCRIPT_DIR}"

BLB_MAKE=`which make`
BLB_MAKE_TARGET="mega2560"
BLB_HEXFILE="stk500boot_v2_mega2560.hex"
BLB_OUTFILE_PREFIX="stk500boot_v2_mega2560"
BLB_OUTFILE_POSTFIX=".hex"

echo "Build Directory: $BLB_BUILD_DIR"
echo "Output Directory: $BLB_OUT_DIR"

if [ ! -e "$BLB_OUT_DIR" ]; then
  mkdir -p "$BLB_OUT_DIR" || {
    echo "ERROR: Could not create target directory: $BLB_OUT_DIR" >&2
    exit 1
  }
fi

function blbBuildBootloader() {
  BLB_OUTFILE_PATH="$BLB_OUT_DIR/$BLB_OUTFILE_PREFIX-$3$BLB_OUTFILE_POSTFIX"

  $BLB_MAKE clean
  EXTRA_CFLAGS="$2" $BLB_MAKE "$1"

  if [ ! -e "$BLB_HEXFILE" ]; then
    echo "ERROR: $BLB_HEXFILE doesn't exist!" >&2
    exit 1
  fi

  if [ -e "$BLB_OUTFILE_PATH" ]; then
    rm "$BLB_OUTFILE_PATH" || {
      echo "ERROR: Could not delete: $BLB_OUTFILE_PATH" >&2
      exit 1
    }
  fi
  mv "$BLB_HEXFILE" "$BLB_OUTFILE_PATH"

}

cd $BLB_BUILD_DIR

# Build standard bootloader
blbBuildBootloader "mega2560" "-DNO_DEFAULTS" "standard"

# Build standard + VSELECT
blbBuildBootloader "mega2560" "-DNO_DEFAULTS -DENABLE_VSELECT" "vs"

# Build standard + OBMEGA
blbBuildBootloader "mega2560" "-DNO_DEFAULTS -DREMOVE_BOOTLOADER_LED -DENABLE_VSELECT -DENABLE_ONBOARD_ATMEGA" "obm"

# Build minimal
blbBuildBootloader "mega2560xs" "-DNO_DEFAULTS -DREMOVE_MONITOR -DREMOVE_PROGRAM_LOCK_BIT_SUPPORT -DREMOVE_BOOTLOADER_LED" "min"

# Build minimal + VSELECT
blbBuildBootloader "mega2560xs" "-DNO_DEFAULTS -DREMOVE_MONITOR -DREMOVE_PROGRAM_LOCK_BIT_SUPPORT -DREMOVE_BOOTLOADER_LED -DENABLE_VSELECT" "min-vs"

# Build minimal + OBMEGA
blbBuildBootloader "mega2560s" "-DNO_DEFAULTS -DREMOVE_MONITOR -DREMOVE_PROGRAM_LOCK_BIT_SUPPORT -DREMOVE_BOOTLOADER_LED -DENABLE_VSELECT -DENABLE_ONBOARD_ATMEGA" "min-obm"

$BLB_MAKE clean

exit 0
