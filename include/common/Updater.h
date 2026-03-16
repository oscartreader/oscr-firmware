/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_UPDATER_H_
#define OSCR_UPDATER_H_

namespace OSCR
{
  namespace Updater
  {
    /**
     * Outputs the version & feature string to the serial console.
     */
    extern void printVersionToSerial();

    extern bool execCommand();

    extern void check();
  } // namespace Updater
} // namespace OSCR

#endif/* OSCR_UPDATER_H_ */
