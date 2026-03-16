/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/

#include "config.h"

#if defined(OSCR_ARCH_AVR) && defined(ENABLE_PINCONTROL)

# include "arch/avr/syslibinc.h"
# include "common/OSCR.h"
# include "common/PinControl.h"

namespace OSCR
{
  namespace Hardware
  {
    PinBank getPinBank(uint_fast8_t bank)
    {
      return ((bank >= kPinBankMax) ? PinBank::kPinBankMax : static_cast<PinBank>(bank));
    }

    volatile uint8_t& getPinFromBank(PinBank bank)
    {
      switch(bank)
      {
      case PinBank::kPinBankA: return PINA;
      case PinBank::kPinBankC: return PINC;
      case PinBank::kPinBankF: return PINF;
      case PinBank::kPinBankH: return PINH;
      case PinBank::kPinBankK: return PINK;
      case PinBank::kPinBankL: return PINL;
      case PinBank::kPinBankG: return PING;
      case PinBank::kPinBankMax: break;
      }
      exit(1);
    }

    volatile uint8_t& getDDRFromBank(PinBank bank)
    {
      switch(bank)
      {
      case PinBank::kPinBankA: return DDRA;
      case PinBank::kPinBankC: return DDRC;
      case PinBank::kPinBankF: return DDRF;
      case PinBank::kPinBankH: return DDRH;
      case PinBank::kPinBankK: return DDRK;
      case PinBank::kPinBankL: return DDRL;
      case PinBank::kPinBankG: return DDRG;
      case PinBank::kPinBankMax: break;
      }
      exit(1);
    }

    volatile uint8_t& getPortFromBank(PinBank bank)
    {
      switch(bank)
      {
      case PinBank::kPinBankA: return PORTA;
      case PinBank::kPinBankC: return PORTC;
      case PinBank::kPinBankF: return PORTF;
      case PinBank::kPinBankH: return PORTH;
      case PinBank::kPinBankK: return PORTK;
      case PinBank::kPinBankL: return PORTL;
      case PinBank::kPinBankG: return PORTG;
      case PinBank::kPinBankMax: break;
      }
      exit(1);
    }
  }
}

#endif
