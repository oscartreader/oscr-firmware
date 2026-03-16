
#if !defined(OSCR_CORE_MACROS_H_)
#define OSCR_CORE_MACROS_H_

#include "macros.h"

#define OSCR_CORES(...) \
  enum class CoreID : uint8_t\
  {\
    NONE,\
    __VA_ARGS__\
  }

#define REGISTER_CORE_MENU2(TITLE, MENU) \
  { \
    .title = TITLE, \
    .menu = &MENU, \
  }

#define REGISTER_CORE_MENU3(ID, TITLE, MENU) \
  { \
    .id = CoreID::ID, \
    .title = TITLE, \
    .menu = &MENU, \
  }

#define REGISTER_CORE_ID2(TITLE, MENU)

#define REGISTER_CORE_ID3(ID, TITLE, MENU) ID,

#endif /* OSCR_CORE_MACROS_H_ */

#if defined(OSCR_REGISTERING_CORE_IDS)
# define REGISTER_CORE(...) GET_MACRO_0_3(REGISTER_CORE_ID, __VA_ARGS__)
#elif defined(OSCR_REGISTERING_CORE_MENUS)
# define REGISTER_CORE(...) GET_MACRO_0_3(REGISTER_CORE_MENU, __VA_ARGS__)
#else
# define REGISTER_CORE(...)
#endif
