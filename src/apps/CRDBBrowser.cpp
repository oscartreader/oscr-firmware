#include "apps/CRDBBrowser.h"

#define CRDB_BROWSER_TEMPLATE \
template < \
  typename CRDBType, \
  typename RecordType, \
  uint_fast32_t dbRecordSize, \
  typename OSCR::Util::enable_if_t<( \
    OSCR::Util::is_base_of_template<OSCR::CRDB::CRDBNamedByCRC32, CRDBType>::value || \
    OSCR::Util::is_base_of_template<OSCR::CRDB::CRDBNamedByID, CRDBType>::value || \
    OSCR::Util::is_base_of_template<OSCR::CRDB::CRDBNamedByMapperSubmapper, CRDBType>::value \
  ), bool> EnableDB \
>

namespace OSCR
{
  namespace Apps
  {
    CRDB_BROWSER_TEMPLATE
    OSCR::Apps::CRDBBrowser<CRDBType, RecordType, dbRecordSize, EnableDB>::CRDBBrowser(char const * browserTitle, CRDBType * crdbTarget)
      : MenuRenderer(browserTitle, crdbTarget->numRecords()),
        crdb(crdbTarget)
    {
      setup();
    }

    CRDB_BROWSER_TEMPLATE
    OSCR::Apps::CRDBBrowser<CRDBType, RecordType, dbRecordSize, EnableDB>::CRDBBrowser(__FlashStringHelper const * browserTitle, CRDBType * crdbTarget)
      : MenuRenderer(browserTitle, crdbTarget->numRecords()),
        crdb(crdbTarget)
    {
      setup();
    }

    CRDB_BROWSER_TEMPLATE
    void OSCR::Apps::CRDBBrowser<CRDBType, RecordType, dbRecordSize, EnableDB>::setup()
    {
      onPageChange();
    }

    CRDB_BROWSER_TEMPLATE
    void OSCR::Apps::CRDBBrowser<CRDBType, RecordType, dbRecordSize, EnableDB>::onPageChange()
    {
      uint16_t indexOffset = getPageEntryOffset();

      for (uint_fast16_t menuIndex = 0; menuIndex < getPageEntryCount(); ++menuIndex)
      {
        uint16_t entryIndex = indexOffset + menuIndex + 1;

        crdb->loadRecordIndex(entryIndex);

        strlcpy(pageEntries[menuIndex], crdb->record()->data()->name, UI_PAGE_ENTRY_LENTH_MAX);
      }
    }

    CRDB_BROWSER_TEMPLATE
    bool OSCR::Apps::CRDBBrowser<CRDBType, RecordType, dbRecordSize, EnableDB>::onConfirm()
    {
      crdb->loadRecordIndex(getEntryIndex() + 1);

      return true;
    }

    template class CRDBBrowser<OSCR::Databases::Basic::CRDatabase, OSCR::Databases::Basic::BasicRecord, CRDB_RECORD_SIZE_BASIC>;
    template class CRDBBrowser<OSCR::Databases::Standard::CRDatabase, OSCR::Databases::Standard::StandardRecord, CRDB_RECORD_SIZE_STANDARD>;
    template class CRDBBrowser<OSCR::Databases::Extended::CRDatabase, OSCR::Databases::Extended::ExtendedRecord, CRDB_RECORD_SIZE_EXTENDED>;

    template class CRDBBrowser<OSCR::Databases::Basic::CRMapperDatabase, OSCR::Databases::Basic::MapperRecord, CRDB_RECORD_SIZE_BASIC_MAPPER>;

# if HAS_NES
    template class CRDBBrowser<OSCR::Databases::NES, OSCR::Databases::NESRecord, CRDB_RECORD_SIZE_NES>;
    template class CRDBBrowser<OSCR::Databases::NESMapper, OSCR::Databases::NESMapperRecord, CRDB_RECORD_SIZE_NES_MAPPER>;
# endif /* HAS_NES */

# if HAS_SNES
    template class CRDBBrowser<OSCR::Databases::SNES, OSCR::Databases::SNESRecord, CRDB_RECORD_SIZE_SNES>;
# endif /* HAS_SNES */

# if HAS_TI99
    template class CRDBBrowser<OSCR::Databases::TI99, OSCR::Databases::TI99Record, CRDB_RECORD_SIZE_TI99>;
# endif /* HAS_TI99 */
  }

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Basic::CRDatabase * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::Basic::CRDatabase, OSCR::Databases::Basic::BasicRecord, CRDB_RECORD_SIZE_BASIC>(title, crdbTarget);

    browser.select();
  }

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Basic::CRMapperDatabase * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::Basic::CRMapperDatabase, OSCR::Databases::Basic::MapperRecord, CRDB_RECORD_SIZE_BASIC_MAPPER>(title, crdbTarget);

    browser.select();
  }

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Standard::CRDatabase * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::Standard::CRDatabase, OSCR::Databases::Standard::StandardRecord, CRDB_RECORD_SIZE_STANDARD>(title, crdbTarget);

    browser.select();
  }

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Extended::CRDatabase * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::Extended::CRDatabase, OSCR::Databases::Extended::ExtendedRecord, CRDB_RECORD_SIZE_EXTENDED>(title, crdbTarget);

    browser.select();
  }

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Flash::Flash * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::Flash::Flash, OSCR::Databases::Flash::FlashRecord, CRDB_RECORD_SIZE_FLASH>(title, crdbTarget);

    browser.select();
  }

# if HAS_NES
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::NES * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::NES, OSCR::Databases::NESRecord, CRDB_RECORD_SIZE_NES>(title, crdbTarget);

    browser.select();
  }

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::NESMapper * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::NESMapper, OSCR::Databases::NESMapperRecord, CRDB_RECORD_SIZE_NES_MAPPER>(title, crdbTarget);

    browser.select();
  }
# endif /* HAS_NES */

# if HAS_SNES
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::SNES * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::SNES, OSCR::Databases::SNESRecord, CRDB_RECORD_SIZE_SNES>(title, crdbTarget);

    browser.select();
  }
# endif /* HAS_SNES */

# if HAS_TI99
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::TI99 * crdbTarget)
  {
    Apps::CRDBBrowser browser = Apps::CRDBBrowser<OSCR::Databases::TI99, OSCR::Databases::TI99Record, CRDB_RECORD_SIZE_TI99>(title, crdbTarget);

    browser.select();
  }
# endif /* HAS_TI99 */
}
