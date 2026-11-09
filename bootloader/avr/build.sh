#!/usr/bin/env bash

BLB_ORIGINAL_CWD=`pwd`
BLB_SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BLB_STK500_SRC_DIR="$BLB_SCRIPT_DIR/stk500v2/src"
BLB_URBOOT_SRC_DIR="$BLB_SCRIPT_DIR/urboot"
BLB_OUT_DIR="${BLB_OUT_DIR-$BLB_SCRIPT_DIR}"

BLB_MAKE=`which make`
BLB_MAKE_TARGET="mega2560"

BLB_STK500_HEXFILE="stk500boot_v2_mega2560.hex"
BLB_STK500_OUTFILE_PREFIX="stk500v2"

BLB_URBOOT_HEXFILE="urboot.hex"
BLB_URBOOT_OUTFILE_PREFIX="urboot"

BLB_OUTFILE_POSTFIX=".hex"

BLB_B_STK500="default"
BLB_B_URBOOT="default"

BLB_B_ALL="yes"
BLB_B_GENERIC="default"
BLB_B_RRR="default"
BLB_B_STARSHADE="default"
BLB_B_EXPERIMENTAL="default"

PARAM_COUNT=${#}
i="1"

while [[ "$i" -le $PARAM_COUNT ]]; do
  PARAM="${!i}"
  if [[ "$PARAM" = "-h" ]] || [[ "$PARAM" = "--help" ]]; then
    echo "Usage: $0 [options] [...type(s)]"
    echo "  --arduino | -a  : Build only Arduino (STK500) bootloaders."
    echo "  --stk500"
    echo ""
    echo "  --urboot  | -u  : Build only urboot bootloaders."
    echo ""
    echo "Valid Types:"
    echo "  * generic"
    echo "  * radrabit"
    echo "  * starshade"
    echo "  * experimental"
    exit 0
  elif [[ "$PARAM" = "--stk500" ]] || [[ "$PARAM" = "--arduino" ]] || [[ "$PARAM" = "-a" ]]; then
    BLB_B_STK500="yes"
    if [[ "$BLB_B_URBOOT" == "default" ]]; then
      BLB_B_URBOOT="no"
    fi
  elif [[ "$PARAM" = "--urboot" ]] || [[ "$PARAM" = "-u" ]]; then
    BLB_B_URBOOT="yes"
    if [[ "$BLB_B_STK500" == "default" ]]; then
      BLB_B_STK500="no"
    fi
  elif [[ "$PARAM" = "generic" ]]; then
    BLB_B_GENERIC="yes"
    BLB_B_ALL="no"
  elif [[ "$PARAM" = "radrabit" ]]; then
    BLB_B_RRR="yes"
    BLB_B_ALL="no"
  elif [[ "$PARAM" = "starshade" ]]; then
    BLB_B_STARSHADE="yes"
    BLB_B_ALL="no"
  elif [[ "$PARAM" = "experimental" ]]; then
    BLB_B_EXPERIMENTAL="yes"
    BLB_B_ALL="no"
  else
    echo "Unknown option: ${!i}"
    exit 1
  fi
  i=$[$i + 1]
done

GCCROOT=$( readlink -f "$BLB_SCRIPT_DIR/../../toolchain/avr-gcc/bin/" )
AVR_INC=$( readlink -f "$BLB_SCRIPT_DIR/../../toolchain/avr-gcc/avr/include" )

echo "STK500v2 Directory: $BLB_STK500_SRC_DIR"
echo "urboot Directory: $BLB_URBOOT_SRC_DIR"
echo "Output Directory: $BLB_OUT_DIR"

if [ ! -z "$GCCROOT" ]; then
  TOOLVER="system"
  echo "GCC Directory: $GCCROOT ($TOOLVER)"
  PATH="$GCCROOT:$PATH"
fi

if [ ! -e "$BLB_OUT_DIR" ]; then
  mkdir -p "$BLB_OUT_DIR" || {
    echo "ERROR: Could not create target directory: $BLB_OUT_DIR" >&2
    exit 1
  }
fi

function blbBuildSTKBootloader() {
  BLB_OUTFILE_PATH="$BLB_OUT_DIR/$BLB_STK500_OUTFILE_PREFIX-$3$BLB_OUTFILE_POSTFIX"

  $BLB_MAKE clean
  EXTRA_CFLAGS="$2" $BLB_MAKE "$1"

  if [ ! -e "$BLB_STK500_HEXFILE" ]; then
    echo "ERROR: $BLB_STK500_HEXFILE doesn't exist!" >&2
    exit 1
  fi

  if [ -e "$BLB_OUTFILE_PATH" ]; then
    rm "$BLB_OUTFILE_PATH" || {
      echo "ERROR: Could not delete: $BLB_OUTFILE_PATH" >&2
      exit 1
    }
  fi
  mv "$BLB_STK500_HEXFILE" "$BLB_OUTFILE_PATH"
}

function blbBuildUrbootBootloader() {
  BLB_OUTFILE_NAME="$BLB_URBOOT_OUTFILE_PREFIX-$1"
  BLB_BAUD=$2
  BLB_VSELECT=$3
  BLB_OBMEGA=$4
  BLB_LED="AtmelPB7"

  if [ ! -z "$6" ]; then
    BLB_F_CPU="$6"
    if [ ! "$6" -eq "16000000" ]; then
      BLB_F_CPU_MHZ=$(( $BLB_F_CPU / 1000000 ))
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-${BLB_F_CPU_MHZ}mhz"
    fi
  else
    BLB_F_CPU="16000000"
  fi

  if [ "$BLB_OBMEGA" -gt "0" ]; then
    if [ "$BLB_VSELECT" -gt "1" ]; then
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-obm$BLB_VSELECT"
    elif [ "$BLB_OBMEGA" -gt "1" ]; then
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-obm$BLB_OBMEGA"
    else
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-obm"
    fi
    BLB_LED="AtmelPD5"
  elif [ "$BLB_VSELECT" -gt "0" ]; then
    if [ "$BLB_VSELECT" -eq "1" ]; then
      BLB_VSELECT=3
    fi
    BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-vs$BLB_VSELECT"
  fi

  BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-${BLB_BAUD}bps"

  if [ ! -z "$7" ]; then
    if   [ "$7" -gt "999" ]; then
      BLB_WDTON=$(( $7 / 1000 ))
      BLB_WDTO="${BLB_WDTON}S"
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-${BLB_WDTON}s"
    elif [ "$7" -gt "16" ]; then
      BLB_WDTO="${7}MS"
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-${7}ms"
    elif [ "$7" -lt "16" ]; then
      BLB_WDTO="${7}S"
      BLB_OUTFILE_NAME="$BLB_OUTFILE_NAME-${7}s"
    else
      echo "Unknown value for WDTO"
      exit 1
    fi
  else
    BLB_WDTO="500MS"
  fi

  BLB_OUTFILE_PATH="$BLB_OUT_DIR/$BLB_OUTFILE_NAME$BLB_OUTFILE_POSTFIX"

  GCCROOT="$GCCROOT/" TOOLVER="$TOOLVER" $BLB_MAKE clean
  GCCROOT="$GCCROOT/" TOOLVER="$TOOLVER" $BLB_MAKE MCU=atmega2560 F_CPU=${BLB_F_CPU}U BAUD_RATE=$BLB_BAUD WDTO=${BLB_WDTO} PGMWRITEPAGE=1 UPDATE_FL=4 CHIP_ERASE=1 EXITFE=2 BLINK=1 LED=$BLB_LED ENABLE_VSELECT=$BLB_VSELECT ENABLE_ONBOARD_ATMEGA=$BLB_OBMEGA NAME=$BLB_URBOOT_OUTFILE_PREFIX $5

  if [ ! -e "$BLB_URBOOT_HEXFILE" ]; then
    echo "ERROR: $BLB_URBOOT_HEXFILE doesn't exist!" >&2
    exit 1
  fi

  if [ -e "$BLB_OUTFILE_PATH" ]; then
    rm "$BLB_OUTFILE_PATH" || {
      echo "ERROR: Could not delete: $BLB_OUTFILE_PATH" >&2
      exit 1
    }
  fi
  mv "$BLB_URBOOT_HEXFILE" "$BLB_OUTFILE_PATH"
}


#
# ===== STK500V2 BOOTLOADERS =====
# Only use these if you want to keep compatibility with the Arduino IDE for some reason.
#
# Params: <target> <flags> <name>
#

if [[ "$BLB_B_STK500" != "no" ]]; then

  cd "$BLB_STK500_SRC_DIR"

  # Build standard bootloader
  blbBuildSTKBootloader "mega2560" "-DNO_DEFAULTS -I$AVR_INC" "generic"

  # Build standard + VSELECT
  blbBuildSTKBootloader "mega2560" "-DNO_DEFAULTS -DENABLE_VSELECT=3 -I$AVR_INC" "generic-vs3"

  # OBMEGA version is using minimal to stay away from the max size
  blbBuildSTKBootloader "mega2560s" "-DNO_DEFAULTS -DREMOVE_MONITOR -DREMOVE_PROGRAM_LOCK_BIT_SUPPORT -DREMOVE_BOOTLOADER_LED -DENABLE_VSELECT=1 -DENABLE_ONBOARD_ATMEGA=5 -I$AVR_INC" "generic-obm5"

  $BLB_MAKE clean
fi

#
# ===== URBOOT BOOTLOADERS =====
# You should be using one of these if you are burning a bootloader anyway.
#

#
# Params: "<name>" <baud> <Vselect> <integrated mega> "<extra flags>" [ "WDTO" [ "F_CPU" ] ]
#
# name                  : name, only affects the file name (usually "generic", "experimental", or the name of a builder)
#
# baud                  : The baud rate that will be used when flashing the firmware.
#                           Note: This is NOT the baud rate for the firmware, they can be different)
#
# vselect flag          : 0 for disabled; 3 for a 3.3V start; 5 for a 5V start
#
# integrated mega flag  : 0 if using a module; 1 for using an integrated mega
#
# extra flags           : Additional flags to pass on to the build script; generic ones have the ident as an example.
#
# WDTO                  : How long to wait (in ms) for an update after an external reset. (Optional, default is 500)
#                           Note: The only reason to increase this is because you disabled automatic reset and you need time after
#                                 manually resetting to start the firmware upload. (i.e. because you are using serial control).
#
# F_CPU                 : The CPU/MCU speed in MHz (usually 16000000)
#                           Note: This alone is not enough to change the clock speed. This is not a 1-button overclock. It also
#                                 requires hardware modification and adjusting some other settings.
#

if [[ "$BLB_B_URBOOT" != "no" ]]; then

  cd "$BLB_URBOOT_SRC_DIR"

  if [[ "$BLB_B_ALL" == "yes" ]] || [[ $BLB_B_GENERIC == "yes" ]]; then
    blbBuildUrbootBootloader "generic"  250000 0 0 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic"  250000 3 0 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic"  250000 3 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic"  250000 5 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic" 1000000 3 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic" 1000000 5 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic" 2000000 5 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00"
    blbBuildUrbootBootloader "generic"  250000 5 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00" "16000000" 1000
    blbBuildUrbootBootloader "generic" 1000000 5 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00" "16000000" 1000
  fi

  if [[ "$BLB_B_ALL" == "yes" ]] || [[ $BLB_B_RRR == "yes" ]]; then
    blbBuildUrbootBootloader "radrabit-hw5r5"         250000 3 0 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=5 OSCR_IDENT_BUILDER=0x01 OSCR_IDENT_SLOTS=0x7F OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x87"
    blbBuildUrbootBootloader "radrabit-hw5r8"         250000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x01 OSCR_IDENT_SLOTS=0x7F OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
    blbBuildUrbootBootloader "radrabit-hw5r8-gg"      250000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x01 OSCR_IDENT_SLOTS=0xFF OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
  fi

  if [[ "$BLB_B_ALL" == "yes" ]] || [[ $BLB_B_STARSHADE == "yes" ]]; then
    blbBuildUrbootBootloader "starshade-hw5r8-nogg"   250000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x69 OSCR_IDENT_SLOTS=0xFF OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
    blbBuildUrbootBootloader "starshade-hw5r8"        250000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x69 OSCR_IDENT_SLOTS=0xFF OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
    blbBuildUrbootBootloader "starshade-hw5r8"       1000000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x69 OSCR_IDENT_SLOTS=0xFF OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
    blbBuildUrbootBootloader "starshade-hw5r8"       2000000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x69 OSCR_IDENT_SLOTS=0xFF OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
    blbBuildUrbootBootloader "starshade-kit-hw5r8"    250000 5 1 "OSCR_IDENT_HW_VERSION=5 OSCR_IDENT_HW_REVISION=8 OSCR_IDENT_BUILDER=0x6A OSCR_IDENT_SLOTS=0x7F OSCR_IDENT_INTEGRATED=0xEF OSCR_IDENT_HARDWARE=0x87"
  fi

  if [[ "$BLB_B_ALL" == "yes" ]] || [[ $BLB_B_EXPERIMENTAL == "yes" ]]; then
    blbBuildUrbootBootloader "experimental" 250000 5 1 "OSCR_IDENT_HW_VERSION=0 OSCR_IDENT_HW_REVISION=0 OSCR_IDENT_BUILDER=0x00 OSCR_IDENT_SLOTS=0x00 OSCR_IDENT_INTEGRATED=0x00 OSCR_IDENT_HARDWARE=0x00" "20000000"
  fi

  $BLB_MAKE clean
fi

exit 0
