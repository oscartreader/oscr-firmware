/**
 * @file
 * @brief OSCR File Browser application
 */
#pragma once
#ifndef OSCR_CRDB_BROWSER_H_
# define OSCR_CRDB_BROWSER_H_

# include "config.h"

# include "common/specializations.h"
# include "common/Types.h"
# include "common/Util.h"
# include "common/CRDB.h"
# include "common/crdb/basic.h"
# include "common/crdb/standard.h"
# include "common/crdb/extended.h"
# include "common/crdb/mapper.h"
# include "common/crdb/nes.h"
# include "common/crdb/snes.h"
# include "common/crdb/gba.h"
# include "common/crdb/ti99.h"
# include "common/crdb/flash.h"
# include "ui.h"

namespace OSCR
{
  namespace Apps
  {
    /**
     * @class CRDBBrowser
     *
     * @brief An interface for browsing CRDB entries.
     *
     * Create a CRDB browser interface.
     *
     * @note
     * Available memory on embedded microcontrollers is limited. You
     * should consider using `OSCR::crdbBrowser()` instead of implementing
     * this class directly.
     *
     * @tparam RecordStruct   The data record struct
     * @tparam dbRecordSize   The size of a data record
     * @tparam RecordType     The record's class instance
     * @tparam CRDBType       The CRDB class instance
     */
    template <
      typename CRDBType,
      typename RecordType,
      uint_fast32_t dbRecordSize,
      typename OSCR::Util::enable_if_t<(
        OSCR::Util::is_base_of_template<OSCR::CRDB::CRDBNamedByCRC32, CRDBType>::value ||
        OSCR::Util::is_base_of_template<OSCR::CRDB::CRDBNamedByID, CRDBType>::value || \
        OSCR::Util::is_base_of_template<OSCR::CRDB::CRDBNamedByMapperSubmapper, CRDBType>::value \
      ), bool> EnableDB = true
    >
    class CRDBBrowser : public OSCR::UI::MenuRenderer
    {
    public:
      /**
       * Create a CRDB browser interface.
       *
       * @param browserTitle  A flash string to use for the menu title.
       */
      CRDBBrowser(char const * browserTitle, CRDBType * crdbTarget);
      CRDBBrowser(__FlashStringHelper const * browserTitle, CRDBType * crdbTarget);

    protected:
      //! @internal
      CRDBType * crdb;

      void setup();
      void onPageChange();
      bool onConfirm();

      //void gotoRecordIndex(uint16_t entryIndex);
      //char * getName();
    };
  } /* namespace Apps */

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Basic::CRDatabase * crdbTarget);
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Basic::CRMapperDatabase * crdbTarget);
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Standard::CRDatabase * crdbTarget);
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Extended::CRDatabase * crdbTarget);

  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::Flash::Flash * crdbTarget);

# if HAS_NES
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::NES * crdbTarget);
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::NESMapper * crdbTarget);
# endif /* HAS_NES */

# if HAS_SNES
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::SNES * crdbTarget);
# endif /* HAS_SNES */

# if HAS_TI99
  void crdbBrowser(__FlashStringHelper const * title, OSCR::Databases::TI99 * crdbTarget);
# endif /* HAS_TI99 */

} /* namespace OSCR */

#endif /* OSCR_CRDB_BROWSER_H_ */
