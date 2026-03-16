/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_PROMPTS_H_
#define OSCR_PROMPTS_H_

#include "syslibinc.h"
#include "config.h"

namespace OSCR::Prompts
{
  enum class AbortRetryContinue : uint8_t
  {
    Abort,
    Retry,
    Continue,
  };

  AbortRetryContinue abortRetryContinue();
  bool confirmErase();
  bool askYesNo(char const * question);
} /* namespace OSCR::Prompts */

#endif/* OSCR_PROMPTS_H_ */
