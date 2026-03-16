
#pragma once
#ifndef CRDB_NES_H_
# define CRDB_NES_H_

# include "common/CRDB.h"
# include "cores/NES.h"

namespace OSCR::Databases
{
  using OSCR::Cores::NES::crdbNESRecord;
  using OSCR::Cores::NES::crdbNESMapperRecord;

  class NESMapperRecord
    : public OSCR::CRDB::GenericRecord<crdbNESMapperRecord>
  {
      void debug()
      {
#   if defined(ENABLE_CRDB_DEBUG)
        OSCR::Serial::printLine(F(" ===== ROM DEBUG INFO ===== "));

        OSCR::Serial::print(FS(OSCR::Strings::Labels::MAPPER));
        OSCR::Serial::printLine(_data.mapper);

        OSCR::Serial::print(FS(OSCR::Strings::Labels::SUBMAPPER));
        OSCR::Serial::printLine(_data.submapper);

        OSCR::Serial::print(FS(OSCR::Strings::Common::Lower));
        OSCR::Serial::print(FS(OSCR::Strings::Common::PRG));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::Serial::printLine(_data.prglo);

        OSCR::Serial::print(FS(OSCR::Strings::Common::Upper));
        OSCR::Serial::print(FS(OSCR::Strings::Common::PRG));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::Serial::printLine(_data.prghi);

        OSCR::Serial::print(FS(OSCR::Strings::Common::Lower));
        OSCR::Serial::print(FS(OSCR::Strings::Common::CHR));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::Serial::printLine(_data.chrlo);

        OSCR::Serial::print(FS(OSCR::Strings::Common::Upper));
        OSCR::Serial::print(FS(OSCR::Strings::Common::CHR));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::Serial::printLine(_data.chrhi);

        OSCR::Serial::print(FS(OSCR::Strings::Common::Lower));
        OSCR::Serial::print(FS(OSCR::Strings::Common::RAM));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::Serial::printLine(_data.ramlo);

        OSCR::Serial::print(FS(OSCR::Strings::Common::Upper));
        OSCR::Serial::print(FS(OSCR::Strings::Common::RAM));
        OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
        OSCR::Serial::printLine(_data.ramhi);

        OSCR::Serial::printLine(F(" ========================== "));
#   endif /* ENABLE_CRDB_DEBUG */
      }
  };

  class NESMapper
    : public OSCR::CRDB::CRDB<NESMapperRecord, CRDB_RECORD_SIZE_NES_MAPPER, OSCR::CRDB::CRDBType::ByID>
  {
    using CRDB::CRDB;
    public:
      void nextRecord()
      {
        clearError();
        readNum32(&currentRecord->data()->mapper);
        readNum32(&currentRecord->data()->submapper);
        readNum32(&currentRecord->data()->prglo);
        readNum32(&currentRecord->data()->prghi);
        readNum32(&currentRecord->data()->chrlo);
        readNum32(&currentRecord->data()->chrhi);
        readNum32(&currentRecord->data()->ramlo);
        readNum32(&currentRecord->data()->ramhi);
      }
  };

  class NESRecord
    : public OSCR::CRDB::GenericCartRecord<crdbNESRecord>
  {
  public:
    void debug()
    {
#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLine(F(" ===== ROM DEBUG INFO ===== "));

      OSCR::Serial::print(FS(OSCR::Strings::Labels::NAME));
      OSCR::Serial::printLine(_data.name);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::MAPPER));
      OSCR::Serial::printLine(_data.mapper);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::SUBMAPPER));
      OSCR::Serial::printLine(_data.submapper);

      OSCR::Serial::print(FS(OSCR::Strings::Common::PRG));
      OSCR::Serial::print(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Serial::printLine(_data.prgsize);

      OSCR::Serial::print(FS(OSCR::Strings::Common::PRG));
      OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
      OSCR::Serial::printLine(_data.prg);

      OSCR::Serial::print(FS(OSCR::Strings::Common::CHR));
      OSCR::Serial::print(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Serial::printLine(_data.chrsize);

      OSCR::Serial::print(FS(OSCR::Strings::Common::CHR));
      OSCR::Serial::print(FS(OSCR::Strings::Symbol::LabelEnd));
      OSCR::Serial::printLine(_data.chr);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::RAM_SIZE));
      OSCR::Serial::printLine(_data.ramsize);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::RAM));
      OSCR::Serial::printLine(_data.ram);

      OSCR::Serial::print(F("iNES: "));
      char buffer[9];
      snprintf_P(buffer, 9, PSTR("%02X%02X%02X%02X"), _data.iNES[0], _data.iNES[1], _data.iNES[2], _data.iNES[3]);
      OSCR::Serial::printLine(buffer);

      OSCR::Serial::printLine(F(" ========================== "));
#   endif /* ENABLE_CRDB_DEBUG */
    }
  };

  class NES : public OSCR::CRDB::CRDB<NESRecord, CRDB_RECORD_SIZE_NES>
  {
    using CRDB::CRDB;
  public:
    void nextRecord()
    {
      clearError();
      readNum32(currentRecord->data()->crc32);
      readNum32(currentRecord->data()->id32a);
      readNum32(currentRecord->data()->id32b);
      readNum16(&currentRecord->data()->mapper);
      readNum16(&currentRecord->data()->submapper);
      readNum16(&currentRecord->data()->prgsize);
      readNum16(&currentRecord->data()->chrsize);
      readNum16(&currentRecord->data()->ramsize);
      readBytes(&currentRecord->data()->iNES, 16);
      readBytes(&currentRecord->data()->name, 100);
    }
  };
}

#endif /* CRDB_NES_H_ */
