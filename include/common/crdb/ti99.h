
#pragma once
#ifndef CRDB_TI99_H_
# define CRDB_TI99_H_

# include "config.h"

# if HAS_TI99
#   include "common/CRDB.h"
#   include "cores/TI99.h"

namespace OSCR::Databases
{
  using OSCR::Cores::TI99::crdbTI99Record;

  class TI99Record
    : public OSCR::CRDB::GenericCartRecord<crdbTI99Record>
  {
  public:
    void debug() const
    {
#   if CRDB_DEBUGGING
      OSCR::Serial::printLine(F(" ===== ROM DEBUG INFO ===== "));

      OSCR::Serial::print(FS(OSCR::Strings::Labels::NAME));
      OSCR::Serial::printLine(_data.name);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::CRCSum));
      OSCR::Serial::printLine(_data.crc32);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::MAPPER));
      OSCR::Serial::printLine(_data.mapper);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Serial::printLine(_data.size);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::MAPPER));
      OSCR::Serial::printLine(_data.gmapper);

      OSCR::Serial::print(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Serial::printLine(_data.gsize);

      OSCR::Serial::printLine(F(" ========================== "));
#   endif /* CRDB_DEBUGGING */
    }
  };

  class TI99 : public OSCR::CRDB::CRDB<TI99Record, CRDB_RECORD_SIZE_TI99>
  {
    using CRDB::CRDB;
  public:
    void nextRecord()
    {
      clearError();
      readNum32(currentRecord->data()->crc32);
      readNum16(&currentRecord->data()->mapper);
      readNum16(&currentRecord->data()->size);
      readNum16(&currentRecord->data()->gmapper);
      readNum16(&currentRecord->data()->gsize);
      readBytes(&currentRecord->data()->name, 100);
    }
  };
}

# endif /* HAS_TI99 */

#endif /* CRDB_TI99_H_ */
