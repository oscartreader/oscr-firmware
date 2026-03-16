#pragma once
#if !defined(OSCR_CORE_ATARILYNX_H_)
# define OSCR_CORE_ATARILYNX_H_

# include "config.h"

# if HAS_LYNX
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Atari Lynx
 */
namespace OSCR::Cores::AtariLynx
{
  void setup();
  void menu();
} /* namespace OSCR::Cores::AtariLynx */

# endif /* HAS_LYNX */
#endif /* OSCR_CORE_ATARILYNX_H_ */
