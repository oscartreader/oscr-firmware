/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#if !defined(OSCR_CORES_INC_H_)
# define OSCR_CORES_INC_H_

#include "syslibinc.h"
#include "config.h"
#include "hardware.h"
#include "common.h"
#include "ui.h"
#include "api.h"
#include "apps.h"

namespace OSCR
{
  /**
   * @brief %Cores for supported systems
   */
  namespace Cores
  {
    // Data Direction
    extern __constinit DataDirection dataDir;

    // ROM Size
    extern __constinit uint32_t romSize;

    // Cartridge Size
    extern __constinit uint32_t cartSize;

    // Number of Banks
    extern __constinit uint32_t numBanks;

    // 21 chars for ROM name, one char for termination
    extern uint8_t const kFileNameMax;
    extern __constinit char fileName[];

    extern __constinit uint32_t sramSize;
    extern __constinit uint16_t romType;
    extern __constinit uint8_t saveType;

    extern __constinit char checksumStr[9];
    extern __constinit uint16_t checksum;
    extern __constinit uint8_t romVersion;
    extern __constinit char cartID[5];
    extern __constinit char vendorID[5];
    extern __constinit uint32_t fileSize;
    extern __constinit uint32_t sramBase;

    extern __constinit uint8_t eepbit[8];

    extern crc32_t crc32sum;
    extern __constinit uint32_t writeErrors;

    extern __constinit void * cartCRDB;
    extern __constinit bool fromCRDB;

    // ...

    extern bool useDefaultName();
    extern uint8_t setOutName(char const * const src, uint8_t const srcMaxLen);
    extern uint8_t setOutName_P(char const * const src);
    extern void resetCRDB();
    extern void resetGlobals();
  }
}

#endif /* OSCR_CORES_INC_H_ */
