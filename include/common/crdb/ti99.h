
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
      OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugROM));
      OSCR::Serial::printValue(OSCR::Strings::Common::Name, _data.name);
      OSCR::Serial::printValue(OSCR::Strings::Common::CRCSum, _data.crc32);
      OSCR::Serial::printValue(OSCR::Strings::Common::Mapper, _data.mapper);
      OSCR::Serial::printValue(OSCR::Strings::Common::Size, _data.size);
      OSCR::Serial::printValue(OSCR::Strings::Common::Mapper, _data.gmapper);
      OSCR::Serial::printValue(OSCR::Strings::Common::Size, _data.gsize);
      OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugEnd));
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
