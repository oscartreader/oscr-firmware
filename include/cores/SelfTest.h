#pragma once
#if !defined(OSCR_CORE_SELFTEST_H_)
# define OSCR_CORE_SELFTEST_H_

# include "config.h"

# if HAS_SELFTEST
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief Self test diagnostic assistant.
 */
namespace OSCR::Cores::SelfTest
{
  /**
   * Run the self test diagnostic tool.
   */
  void run();

  /**
   * Run the voltage check diagnostic tool.
   */
  void testVoltages();
}

# endif /* HAS_SELFTEST */
#endif /* OSCR_CORE_SELFTEST_H_ */
