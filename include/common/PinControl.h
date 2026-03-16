/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef PINCONTROL_H_
#define PINCONTROL_H_

#include "config.h"
#include "common/Util.h"

#define BANKSET_MAX_BANKS 4

namespace OSCR
{
  namespace Hardware
  {
    enum PinBank: uint8_t
    {
      kPinBankA = 0,  // [0] Port A
      kPinBankC,      // [1] Port C
      kPinBankF,      // [2] Port F
      kPinBankH,      // [3] Port H
      kPinBankK,      // [4] Port K
      kPinBankL,      // [5] Port L
      // Used pin by pin
      kPinBankG,      // [6] Port G
      kPinBankMax     // Max/Number of banks
    };

    enum class PinDirection : bool
    {
      kIn = 0,
      kOut = 1,

      // Aliases
      kInput = kIn,
      kOutput = kOut,
    };

    enum DataMode: bool
    {
      kRead = 0,
      kWrite = 1,

      // Aliases
      kR = kRead,
      kW = kWrite,
    };

    struct BankSet
    {
      PinBank banks[BANKSET_MAX_BANKS];
      uint8_t width;

      bool addBank(PinBank bank)
      {
        if (bank >= PinBank::kPinBankMax) return false;

        for (uint_fast8_t i = 0; i < BANKSET_MAX_BANKS; ++i)
        {
          if (banks[i] == bank) return true;

          if (banks[i] == PinBank::kPinBankMax)
          {
            banks[i] = bank;
            width++;
            return true;
          }
        }

        return false;
      }

      void reset()
      {
        for (uint_fast8_t i = 0; i < BANKSET_MAX_BANKS; ++i)
        {
          banks[i] = PinBank::kPinBankMax;
        }
        width = 0;
      }

      BankSet()
      {
        reset();
      }
    };

    extern PinBank getPinBank(uint_fast8_t bank);
    extern volatile uint8_t& getPinFromBank(PinBank bank);
    extern volatile uint8_t& getDDRFromBank(PinBank bank);
    extern volatile uint8_t& getPortFromBank(PinBank bank);

    // Bitwise shifts to get the pin number
    constexpr uint8_t const kPinConfigPin1 = 0;
    constexpr uint8_t const kPinConfigPin2 = 1;
    constexpr uint8_t const kPinConfigPin3 = 2;

    // Mask used to extract the pin number
    constexpr uint8_t const kPinConfigMaskPin = (1 << kPinConfigPin1) + (1 << kPinConfigPin2) + (1 << kPinConfigPin3);

    // Bitwise shifts for masking the direction flag
    constexpr uint8_t const kPinConfigDir = 3;

    // Bitwise shifts for masking the pullup flag
    constexpr uint8_t const kPinConfigPullup = 4;

    // Bitwise shifts to get the bank number
    constexpr uint8_t const kPinConfigBank1 = 5;
    constexpr uint8_t const kPinConfigBank2 = 6;
    constexpr uint8_t const kPinConfigBank3 = 7;

    // Mask used to extract the bank number
    constexpr uint8_t const kPinConfigMaskBank = (1 << kPinConfigBank1) + (1 << kPinConfigBank2) + (1 << kPinConfigBank3);

    struct PinConfig
    {
      PinConfig(uint8_t __pin, PinBank __bank, PinDirection __dir, bool __pullup): config(5)
      {
        set(__pin, __bank, __dir, __pullup);
      }

      void set(uint8_t __pin, PinBank __bank, PinDirection __dir, bool __pullup)
      {
        pin(__pin);
        dir(__dir);
        bank(__bank);
        pullup(__pullup);
      }

      //
      // Pin Number

      uint8_t pin()
      {
        return (config[kPinConfigPin1] << 0) | (config[kPinConfigPin2] << 1) | (config[kPinConfigPin3] << 2);
      }

      void pin(uint8_t __pin)
      {
        config[kPinConfigPin1] = (__pin & 0b001) ? true : false;
        config[kPinConfigPin2] = (__pin & 0b010) ? true : false;
        config[kPinConfigPin3] = (__pin & 0b100) ? true : false;
      }

      //
      // Bank Number

      PinBank bank()
      {
        return static_cast<PinBank>((config[kPinConfigBank1] << 0) | (config[kPinConfigBank2] << 1) | (config[kPinConfigBank3] << 2));
      }

      void bank(uint8_t __bank)
      {
        config[kPinConfigBank1] = (__bank & 0b001) ? true : false;
        config[kPinConfigBank2] = (__bank & 0b010) ? true : false;
        config[kPinConfigBank3] = (__bank & 0b100) ? true : false;
      }

      //
      // Data Direction

      PinDirection dir()
      {
        return static_cast<PinDirection>(static_cast<bool>(config[kPinConfigDir]));
      }

      void dir(PinDirection __dir)
      {
        config[kPinConfigDir] = static_cast<bool>(__dir);
      }

      //
      // Pullup

      bool pullup()
      {
        return config[kPinConfigPullup];
      }

      void pullup(bool __pullup)
      {
        config[kPinConfigPullup] = !!__pullup;
      }

    protected:
      OSCR::Util::bitset config;
    };

    struct Pin
    {
      PinConfig config;

      Pin(PinDirection dir, bool pullup, PinBank bank = PinBank::kPinBankMax, uint8_t number = 0): config(number, bank, dir, pullup)
      {
        // ...
      }

      void reset()
      {
        if (config.bank() != PinBank::kPinBankMax)
        {
          if (config.pullup() && config.dir() == PinDirection::kIn && config.pin() < 8)
          {
            // Disable the pullup first
            getPortFromBank(config.bank()) &= ~(1 << config.pin());
          }

          config.bank(PinBank::kPinBankMax);
        }

        config.pin(0);
      }

      void init()
      {
        switch(config.dir())
        {
          case PinDirection::kIn:
            getDDRFromBank(config.bank()) |= 0x00;
            getPortFromBank(config.bank()) = (config.pullup() ? 0xFF : 0x00);

            if (config.pullup()) getPortFromBank(config.bank()) = (1 << config.pin());
            else getPortFromBank(config.bank()) &= ~(1 << config.pin());

            return;

          case PinDirection::kOut:
            getDDRFromBank(config.bank()) = 0xFF;
            getPortFromBank(config.bank()) = 0x00;

            return;
        }
      }

      void setPin(PinBank newBank, uint8_t newNumber)
      {
        reset();

        config.bank(newBank);
        config.pin(newNumber);

        init();
      }

      bool read()
      {
        if (config.bank() >= PinBank::kPinBankMax) return false; // Reading an invalid pin
        return (getPinFromBank(config.bank()) << (config.pin() * 8)) > 0;
      }

      void write(bool state)
      {
        if (config.bank() >= PinBank::kPinBankMax) return; // Writing to an invalid pin
        if (state) getPinFromBank(config.bank()) |= (1 << config.pin());
        else getPinFromBank(config.bank()) &= ~(1 << config.pin());
      }
    };

    class PinControl
    {
      public:
        void reset(bool resetBanks = false, bool resetAllPins = true)
        {
          address->reset();
          data->reset();

          if (resetAllPins)
          {
            clockReadPin->reset();
            clockWritePin->reset();
            latchPin->reset();

            pullup = false;
            clockPeriodWrite = 0;
            clockPeriodRead = 0;
          }

          if (resetBanks)
          {
            for (uint_fast8_t i = 0; i < PinBank::kPinBankMax; ++i)
            {
              setBankDir(getPinBank(i), PinDirection::kIn);
            }
          }
        }

        void usePullup(bool usePullup, bool resetBanks = false)
        {
          pullup = usePullup;
          reset(resetBanks, false);
        }

        void addAddressBank(PinBank bank)
        {
          address->addBank(bank);
          setBankDir(bank, PinDirection::kOut);
        }

        template <typename T, typename ... Ts>
        OSCR::Util::enable_if_t<(OSCR::Util::is_same<T, Ts>::value && ...), void>
        addAddressBank(T bank, Ts ... banks)
        {
          addAddressBank(bank);
          addAddressBank(banks...);
        }

        void addDataBank(PinBank bank)
        {
          data->addBank(bank);
          setBankDir(bank, PinDirection::kIn);
        }

        template <typename T, typename ... Ts>
        OSCR::Util::enable_if_t<(OSCR::Util::is_same<T, Ts>::value && ...), void>
        addDataBank(T bank, Ts ... banks)
        {
          addDataBank(bank);
          addDataBank(banks...);
        }

        void setClockWritePin(PinBank bank, uint8_t pin)
        {
          clockWritePin->setPin(bank, pin);
        }

        void setClockReadPin(PinBank bank, uint8_t pin)
        {
          clockReadPin->setPin(bank, pin);
        }

        void setLatchPin(PinBank bank, uint8_t pin)
        {
          latchPin->setPin(bank, pin);
          useLatchPin = true;
        }

        void waitLatch()
        {
          if (!useLatchPin) return;

          while(!latchPin->read()) NOP;
        }

        void clockRead()
        {
          if (clockPeriodRead > 0)
          {
            clockReadPin->write(true);
            delayMicroseconds(clockPeriodRead);
            clockReadPin->write(false);
          }
        }

        void clockWrite()
        {
          if (clockPeriodWrite > 0)
          {
            clockReadPin->write(true);
            delayMicroseconds(clockPeriodWrite);
            clockReadPin->write(false);
          }
        }

        template <typename T>
        void setAddress(T newAddress)
        {
          uint8_t const size = sizeof(T);

          for (uint8_t i = 0; i < size; i++)
          {
            getPort(address->banks[i]) = (newAddress >> (i * 8)) & 0xFF;
          }
        }

        template <typename T>
        T readData()
        {
          uint8_t const size = sizeof(T);
          T out = 0;

          clockRead();
          waitLatch();

          for (uint8_t i = 0; i < size; i++)
          {
            out |= getPin(data->banks[i]) << (i * 8);
          }

          return out;
        }

        template <typename TD, typename TA>
        TD readAddress(TA newAddress)
        {
          setAddress(newAddress);
          return readData<TD>();
        }

        template <typename TD, typename TA>
        void setPosition(TA newAddress)
        {
          setAddress(newAddress);
          lastAddress = newAddress;
        }

        template <typename TD>
        TD readNext()
        {
          TD read = readData<TD>();
          size_t nextAddress = (data->width * 8) + lastAddress;
          setAddress(nextAddress);
          lastAddress = nextAddress;
          return read;
        }

        template <typename TA>
        TA getPosition()
        {
          return lastAddress;
        }

        template <typename T>
        void writeData(T outputData)
        {
          uint8_t const size = sizeof(T);

          for (uint8_t i = 0; i < size; i++)
          {
            getPort(data->banks[i]) = (outputData >> (i * 8)) & 0xFF;
          }

          clockWrite();
        }

        uint8_t readBank(PinBank bank)
        {
          uint8_t out = 0;
          out |= getPin(bank);
          return out;
        }

        volatile uint8_t& getDDR(PinBank bank)
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

        volatile uint8_t& getPort(PinBank bank)
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

        volatile uint8_t& getPin(PinBank bank)
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

        void setBankDir(PinBank bank, PinDirection dir)
        {
          switch(dir)
          {
            case PinDirection::kIn:
              getDDR(bank) = 0x00;
              getPort(bank) = (pullup ? 0xFF : 0x00);
              return;
            case PinDirection::kOut:
              getDDR(bank) = 0xFF;
              getPort(bank) = 0x00;
              return;
          }
        }

        void setMode(DataMode mode)
        {
          PinDirection pinDir = ((mode == DataMode::kWrite) ? PinDirection::kOut : PinDirection::kIn);

          for (uint8_t i = 0; i < data->width && data->banks[i] != kPinBankMax; i++)
          {
            setBankDir(data->banks[i], pinDir);
          }
        }

      protected:
        BankSet * address = new BankSet;
        BankSet * data = new BankSet;

        Pin * latchPin = new Pin(PinDirection::kIn, false);
        Pin * clockReadPin = new Pin(PinDirection::kOut, false);
        Pin * clockWritePin = new Pin(PinDirection::kOut, false);

        bool useLatchPin = false;
        uint16_t clockPeriodRead = 0;
        uint16_t clockPeriodWrite = 0;

        bool pullup = false;
        size_t lastAddress = 0;
    };
  }
}

#endif /* PINCONTROL_H_ */
