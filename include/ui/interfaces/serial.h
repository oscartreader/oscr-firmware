/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#if !defined(OSCR_UI_SERIAL_H_)
# define OSCR_UI_SERIAL_H_

# include "common/OSCR.h"

# if defined(ENABLE_SERIAL_OUTPUT)
#   include "hardware/outputs/Serial.h"
#   include "ui/interfaces/serial/ascii.h"
#   include "ui/interfaces/serial/ansi.h"
# endif /* ENABLE_SERIAL_OUTPUT */

#endif /* OSCR_UI_SERIAL_H_ */
