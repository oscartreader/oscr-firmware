
#pragma once
#ifndef CRDB_SNES_H_
# define CRDB_SNES_H_

# include "config.h"

# if HAS_SNES
#   include "common/CRDB.h"
#   include "cores/SNES.h"

namespace OSCR::Databases
{
  using OSCR::Cores::SNES::crdbSNESRecord;

  class SNESRecord
    : public OSCR::CRDB::GenericCartRecord<crdbSNESRecord>
  {
  public:
    uint32_t getCRC32()
    {
      return _data.crc32;
    }

    void debug()
    {
#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLine(F(" ===== ROM DEBUG INFO ===== "));

      OSCR::Serial::print(FS(OSCR::Strings::Labels::NAME));
      OSCR::Serial::printLine(_data.name);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::CRCSum));
      OSCR::Serial::printLine(_data.crc32);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::ID));
      OSCR::Serial::printLine(_data.id32);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::CHECKSUM));
      OSCR::Serial::printLine(_data.chksum);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Serial::printLine(_data.size);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::BANKS));
      OSCR::Serial::printLine(_data.banks);

      OSCR::Serial::printLine(F(" ========================== "));
#   endif /* ENABLE_CRDB_DEBUG */
    }
  };

  class SNES : public OSCR::CRDB::CRDB<SNESRecord, CRDB_RECORD_SIZE_SNES>
  {
    using CRDB::CRDB;
  public:
    void nextRecord()
    {
      clearError();
      readNum32(currentRecord->data()->crc32);
      readNum16(&currentRecord->data()->chksum);
      readNum32(currentRecord->data()->id32);
      readNum32(&currentRecord->data()->size);
      readNum16(&currentRecord->data()->banks);
      readBytes(&currentRecord->data()->name, 100);
    }

    bool searchRecord(uint16_t chksumSearch, uint16_t startingRecord = 0)
    {
      constexpr uint8_t const offset = 4;
      clearError();

#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLine(F("<CRDB> Searching..."));
      OSCR::Serial::print(F("<CRDB> Checksum Search: "));
      OSCR::Serial::printHex(chksumSearch);
      OSCR::Serial::printLine();
#   endif /* ENABLE_CRDB_DEBUG */

      for (uint_fast32_t i = startingRecord; gotoRecordIndex(i) && file.seekCur(offset); ++i)
      {
        uint16_t chksum;
        int_fast8_t const readLen = readNum16(&chksum);

        if (readLen != 2) break;

        if (chksum == chksumSearch)
        {
          loadRecordIndex(i);

#   if defined(ENABLE_CRDB_DEBUG)
          OSCR::Serial::print(F("<CRDB> "));
          OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#   endif /* ENABLE_CRDB_DEBUG */

          return true;
        }
      }

#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#   endif /* ENABLE_CRDB_DEBUG */

      gotoRecordIndex(0);
      return false;
    }

    bool searchRecordNext(uint16_t chksumSearch)
    {
      return searchRecord(chksumSearch, recordNum);
    }
  };
}

# endif /* HAS_SNES*/

#endif /* CRDB_SNES_H_ */
