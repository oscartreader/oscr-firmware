/********************************************************************
*                   Open Source Cartridge Reader                    */
/*H******************************************************************
* FILENAME :        CRDB.cpp
*
* DESCRIPTION :
*       Class for the Cartridge Reader Database (CRDB) file format.
*
* PUBLIC FUNCTIONS :
*
* NOTES :
*       This file is a WIP, I've been moving things into it on my local working
*       copy, but they are not ready yet. Rather than put this in the main file
*       only to move them back again, I decided to commit it early. If you need
*       to add new globals, enums, defines, etc, please use this file!
*
* LICENSE :
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* CHANGES :
*
* REF NO    VERSION  DATE        WHO            DETAIL
*           14.5     2024-09-15  Ancyker        Initial version
*
*H*/

#include "common/OSCR.h"
#include "common/CRDB.h"
#include "common/crdb/basic.h"

namespace OSCR::CRDB
{
  constexpr uint32_t  const PROGMEM kDatabaseFileHeader         = 0xDA705C12;
  constexpr uint16_t  const PROGMEM kVersion                    = 1;
  constexpr uint32_t  const PROGMEM kDatestampMinimum           = 0x20240915;
  constexpr char      const PROGMEM PATH_TEMPLATE[]             = "/.oscr/db/%s.crdb";
  constexpr char      const PROGMEM PATH_TEMPLATE_FS[]          = "/.oscr/db/%S.crdb";

  void dbName(char const * databaseName, char databasePath[], uint8_t maxLength)
  {
    if (databaseName[0] == '/') // absolute path (i.e. "/.oscr/db/gb.crdb")
    {
      OSCR::Util::copyStr(databasePath, maxLength, databaseName);
    }
    else // relative path without extension (i.e. "gb")
    {
      snprintf_P(databasePath, maxLength, OSCR::CRDB::PATH_TEMPLATE, databaseName);
    }
  }

  void dbName_P(__FlashStringHelper const * databaseName, char databasePath[], uint8_t maxLength)
  {
    if (pgm_read_byte(databaseName) == '/')
    {
      OSCR::Util::copyStr_P(databasePath, maxLength, databaseName);
    }
    else
    {
      snprintf_P(databasePath, maxLength, OSCR::CRDB::PATH_TEMPLATE_FS, databaseName);
    }
  }

  CRDBBase::CRDBBase(char const * crdb_filename, uint_fast32_t crdb_recordsize)
    : recordSize(crdb_recordsize)
  {
    char databasePath[OSCR::CRDB::kDefaultDatabasePathLength];
    OSCR::CRDB::dbName(crdb_filename, databasePath);
    openDatabase(databasePath);
  }

  CRDBBase::CRDBBase(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize)
    : recordSize(crdb_recordsize)
  {
    char databasePath[OSCR::CRDB::kDefaultDatabasePathLength];
    OSCR::CRDB::dbName_P(crdb_filename, databasePath);
    openDatabase(databasePath);
  }

  void CRDBBase::openDatabase(char const * crdb_filename)
  {
    OSCR::Storage::sd.chdir();
    OSCR::Storage::sd.chdir("/.oscr/db");

    if (!file.open(crdb_filename, O_READ))
    {
      error(E_DATABASE_NOT_FOUND);
#if defined(NEEDS_CRDB_ISQUIET)
      if (isQuiet()) return;
#endif
      OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::CRDB));
      OSCR::UI::fatalError(FS(OSCR::Strings::Errors::DatabaseNotFound));
      return;
    }

    if (!file.available())
    {
#if CRDB_DEBUGGING
      OSCR::Serial::printLine(F("File not available."));
#endif /* CRDB_DEBUGGING */
      databaseError();
      return;
    }

    int32_t readLen = 0;
    uint32_t fileHeader, fileRecordSize, datestamp;
    uint16_t fileVersion;

    readLen = readNum32(&fileHeader);
    if (readLen != 4 || fileHeader != kDatabaseFileHeader)
    {
#if CRDB_DEBUGGING
      OSCR::Serial::printLine(F("Header is incorrect."));
      OSCR::Serial::printLine(readLen);
      OSCR::Serial::printLine(fileHeader);
      OSCR::Serial::printLine(kDatabaseFileHeader);
#endif /* CRDB_DEBUGGING */
      databaseError();
      return;
    }

    readLen = readNum16(&fileVersion);
    if (readLen != 2 || fileVersion != kVersion)
    {
#if CRDB_DEBUGGING
      OSCR::Serial::printLine(F("Version is incorrect."));
      OSCR::Serial::printLine(readLen);
      OSCR::Serial::printLine(fileVersion);
#endif /* CRDB_DEBUGGING */
      databaseError();
      return;
    }

    file.seekCur(19); // Skip NUL byte + 18-character string

    readLen = readNum32(&datestamp);
    if (readLen != 4 || datestamp < kDatestampMinimum)
    {
#if CRDB_DEBUGGING
      OSCR::Serial::printLine(F("Datestamp is incorrect."));
      OSCR::Serial::printLine(readLen);
      OSCR::Serial::printLine(datestamp);
#endif /* CRDB_DEBUGGING */
      databaseError();
      return;
    }

    readLen = readNum32(&fileRecordSize);
    if (readLen != 4 || fileRecordSize != recordSize)
    {
#if CRDB_DEBUGGING
      OSCR::Serial::printLine(F("Record size is incorrect."));
      OSCR::Serial::printLine(readLen);
      OSCR::Serial::printLine(fileRecordSize);
#endif /* CRDB_DEBUGGING */
      databaseError();
      return;
    }

    file.seekSet(recordSize); // Seek to first record

    uint64_t datasize64 = file.available64();
    recordCount = datasize64 / recordSize;
    recordNum = 0;

#if CRDB_DEBUGGING
    uint32_t datasize = (datasize64 > UINT32_MAX ? UINT32_MAX : datasize64);

    OSCR::Serial::printLine(F("Successfully loaded CRDB"));
    OSCR::Serial::print(F("- DB Size: "));
    OSCR::Serial::printLine(datasize);
    OSCR::Serial::print(F("- Record Size: "));
    OSCR::Serial::printLine(recordSize);
    OSCR::Serial::print(F("- Records: "));
    OSCR::Serial::printLine(recordCount);
    OSCR::Serial::flush();
#endif /* CRDB_DEBUGGING */
  }

  CRDBBase::~CRDBBase()
  {
    if (file.isOpen()) file.close();
  }

  void CRDBBase::error(ERRORS code)
  {
    errorCode = code;
  }

  ERRORS CRDBBase::getError() const
  {
    return errorCode;
  }

  bool CRDBBase::hasError() const
  {
    return errorCode != E_NONE;
  }

  void CRDBBase::clearError()
  {
    errorCode = E_NONE;
  }

  void CRDBBase::databaseError()
  {
    error(E_DATABASE_ERROR);
#if defined(NEEDS_CRDB_ISQUIET)
    if (isQuiet()) return;
#endif
    OSCR::UI::printFatalErrorHeader(FS(OSCR::Strings::Headings::CRDB));
    OSCR::UI::fatalError(FS(OSCR::Strings::Errors::DatabaseError));
  }

  int16_t CRDBBase::readBytes(void * dest, uint8_t bytes)
  {
    return file.read(((uint8_t*)dest), bytes);
  }

  uint8_t CRDBBase::readNum8(uint8_t * dest)
  {
    return file.read(((uint8_t*)dest), 1);
  }

  uint8_t CRDBBase::readNum16(uint16_t * dest)
  {
    uint8_t readLen = 0;

    readLen += file.read(((uint8_t*)dest) + 1, 1);
    readLen += file.read(((uint8_t*)dest) + 0, 1);

    return readLen;
  }

  uint8_t CRDBBase::readNum32(uint32_t * dest)
  {
    uint8_t readLen = 0;

    readLen += file.read(((uint8_t*)dest) + 3, 1);
    readLen += file.read(((uint8_t*)dest) + 2, 1);
    readLen += file.read(((uint8_t*)dest) + 1, 1);
    readLen += file.read(((uint8_t*)dest) + 0, 1);

    return readLen;
  }

  uint8_t CRDBBase::readNum32(crc32_t & dest)
  {
    uint8_t readLen = 0;

    readLen += file.read(&dest.b[3], 1);
    readLen += file.read(&dest.b[2], 1);
    readLen += file.read(&dest.b[1], 1);
    readLen += file.read(&dest.b[0], 1);

    return readLen;
  }

  uint_fast16_t CRDBBase::numRecords() const
  {
    return recordCount;
  }

  bool CRDBBase::loadRecordIndex(uint16_t index)
  {
    if (!gotoRecordIndex(index)) return false;

    nextRecord();

    return true;
  }

  bool CRDBBase::gotoRecordIndex(uint16_t index)
  {
    if (index > recordCount) return false;

    recordNum = index;

    return file.seekSet((uint64_t)recordSize * ((uint64_t)1 + (uint64_t)index));
  }

  uint_fast16_t CRDBBase::getRecordIndex()
  {
    return recordNum;
  }


# pragma region ByID

  bool CRDBByIDBase::findRecord(uint16_t const idSearch, uint16_t const offset)
  {
    clearError();

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> Searching..."));

    OSCR::Serial::print(F("<CRDB> CRC32 Search: "));
    OSCR::Serial::printLine(idSearch);
#endif /* CRDB_DEBUGGING */

    for (uint_fast32_t i = 0; gotoRecordIndex(i) && file.seekCur(offset); ++i)
    {
      uint16_t id;
      int_fast8_t const readLen = readNum16(&id);

      if (readLen != 2) break;

      if (id == idSearch)
      {
        loadRecordIndex(i);

#if CRDB_DEBUGGING
        OSCR::Serial::print(F("<CRDB> "));
        OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#endif /* CRDB_DEBUGGING */

        return true;
      }
    }

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#endif /* CRDB_DEBUGGING */

    gotoRecordIndex(0);
    return false;
  }

# pragma region ByMapper

bool CRDBByMapperSubmapperBase::findRecord(uint32_t const mapperSearch, uint32_t const submapperSearch, uint16_t const offset)
  {
    clearError();

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> Searching..."));

    OSCR::Serial::print(F("<CRDB> Mapper Search: "));
    OSCR::Serial::printLine(mapperSearch);
    OSCR::Serial::print(F("<CRDB> Submapper Search: "));
    OSCR::Serial::printLine(submapperSearch);
#endif /* CRDB_DEBUGGING */

    for (uint_fast32_t i = 0; gotoRecordIndex(i) && file.seekCur(offset); ++i)
    {
      uint32_t mapper;
      uint32_t submapper;
      int_fast8_t readLen = 0;

      readLen += readNum32(&mapper);
      readLen += readNum32(&submapper);

      if (readLen != 8) break;

      if ((mapper == mapperSearch) && (submapper == submapperSearch))
      {
        loadRecordIndex(i);

#if CRDB_DEBUGGING
        OSCR::Serial::print(F("<CRDB> "));
        OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#endif /* CRDB_DEBUGGING */

        return true;
      }
    }

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#endif /* CRDB_DEBUGGING */

    gotoRecordIndex(0);
    return false;
  }


# pragma region NamedByCRC32

  bool CRDBNamedByCRC32Base::matchCRC(uint32_t crc32, int dbOffset)
  {
    crc32_t crc32search = crc32_t(crc32);
    return matchCRC(crc32search, dbOffset);
  }

  bool CRDBNamedByCRC32Base::matchCRC(crc32_t & crc32, int dbOffset)
  {
    return matchCRC(&crc32, dbOffset);
  }

  bool CRDBNamedByCRC32Base::matchCRC(crc32_t * crc32ptr, int dbOffset)
  {
    clearError();

#if defined(NEEDS_CRDB_ISQUIET)
    if (!isQuiet())
    {
#endif
      OSCR::UI::printSync(FS(OSCR::Strings::Status::CRC32));
#if defined(NEEDS_CRDB_ISQUIET)
    }
#endif

    if (crc32ptr == nullptr)
    {
#if CRDB_DEBUGGING
      OSCR::Serial::printSync(F("<CRDB> CRC32 File: "));
      OSCR::Serial::printLineSync(OSCR::Storage::Shared::sharedFileName);
#endif
      crc32ptr = &OSCR::CRC32::current;
    }

#if CRDB_DEBUGGING
      OSCR::Serial::printSync(F("<CRDB> CRC32 Matching: "));
      OSCR::Serial::printLineSync(*crc32ptr);
#endif

#if defined(NEEDS_CRDB_ISQUIET)
    if (!isQuiet())
    {
#endif
      OSCR::UI::printSync(*crc32ptr);
#if defined(NEEDS_CRDB_ISQUIET)
    }
#endif

    bool found = (cmprCRC32(crc32ptr)) || (findRecord(*crc32ptr, dbOffset) && !hasError());

#if CRDB_DEBUGGING
    OSCR::Serial::printSync(F("<CRDB> Found: "));
    OSCR::Serial::printLineSync(getRecordName());
#endif
#if defined(NEEDS_CRDB_ISQUIET)
    if (!isQuiet())
    {
#endif
      OSCR::UI::printSync(FS(OSCR::Strings::Symbol::Arrow));
#if defined(NEEDS_CRDB_ISQUIET)
    }
#endif

    if (found)
    {
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_POSTMATCHCRC)
      if (!postMatchCRC()) return false;
#endif

      OSCR::UI::printLineSync(FS(OSCR::Strings::Common::OK));

#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_CRCMATCHSUCCESS)
      if (!crcMatchSuccess()) return false;
#endif

      return true;
    }

#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_CRCMATCHFAIL)
    if (!crcMatchFail()) return false;
#endif

#if defined(NEEDS_CRDB_ISQUIET)
    if (!isQuiet())
    {
#endif
      OSCR::UI::printLine(FS(OSCR::Strings::Common::FAIL));
      OSCR::UI::error(FS(OSCR::Strings::Errors::NotFoundDB));
#if defined(NEEDS_CRDB_ISQUIET)
    }
#endif

    return false;
  }

  // CRC match events
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_POSTMATCHCRC)
  virtual bool CRDBBase::postMatchCRC() { return true; }
#endif
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_CRCMATCHFAIL)
  virtual bool CRDBBase::crcMatchFail() { return true; }
#endif
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_CRCMATCHSUCCESS)
  virtual bool CRDBBase::crcMatchSuccess() { return true; }
#endif

  bool CRDBNamedByCRC32Base::findRecord(uint32_t const crc32, uint16_t const offset)
  {
    crc32_t crc32search(crc32);
    return findRecord(crc32search, offset);
  }

  bool CRDBNamedByCRC32Base::findRecord(crc32_t const & crc32search, uint16_t const offset)
  {
    clearError();

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> Searching..."));

    OSCR::Serial::print(F("<CRDB> CRC32 Search: "));
    OSCR::Serial::printLine(crc32search);
#endif /* CRDB_DEBUGGING */

    for (uint_fast32_t i = 0; gotoRecordIndex(i) && file.seekCur(offset); ++i)
    {
      crc32_t crc32;
      int_fast8_t const readLen = readNum32(crc32);

      if (readLen != 4) break;

      if (crc32 == crc32search)
      {
        if (!loadRecordIndex(i)) break;

#if CRDB_DEBUGGING
        OSCR::Serial::print(F("<CRDB> "));
        OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#endif /* CRDB_DEBUGGING */

        return true;
      }
    }

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#endif /* CRDB_DEBUGGING */

    gotoRecordIndex(0);
    return false;
  }

  bool CRDBBase::setNextIndex(SearchParameters * searchParams)
  {
    uint_fast16_t const indexMax = recordCount - 1;
    uint16_t endRecord;

    if (searchParams == nullptr) searchParams = &search;

    if (searchParams->reverse && (searchParams->i == 0))
    {
      searchParams->i = indexMax;
    }
    else if (!searchParams->reverse && (searchParams->i == indexMax))
    {
      searchParams->i = 0;
    }
    else
    {
      searchParams->i += searchParams->step();
    }

    if (searchParams->startingRecord == 0) endRecord = recordCount;
    else if (searchParams->startingRecord == recordCount) endRecord = 0;
    else endRecord = searchParams->startingRecord + -searchParams->step();

    if (searchParams->i == endRecord) return false; // Will infinite loop otherwise

    return true;
  }

#if (((OPTION_CRDB_MULTIMATCH_METHOD) & CRDBID_MATCH_OR_INVERT) || (((OPTION_CRDB_MULTIMATCH_METHOD) & (CRDBID_MATCH_EITHER|CRDBID_MATCH_ALL)) == 0))
#   define MATCH_NORMAL true
#   define MATCH_INVERT true
#elif ((OPTION_CRDB_MULTIMATCH_METHOD) & CRDBID_MATCH_INVERT)
#   define MATCH_NORMAL false
#   define MATCH_INVERT true
#else
#   define MATCH_NORMAL true
#   define MATCH_INVERT false
#endif

  enum class MatchMethod
  {
    Any,
    Either,
    All,
  };

  constexpr MatchMethod const kMatchMethod =
#if (((OPTION_CRDB_MULTIMATCH_METHOD) & (CRDBID_MATCH_EITHER)) == CRDBID_MATCH_EITHER)
    MatchMethod::Either;
#elif (((OPTION_CRDB_MULTIMATCH_METHOD) & (CRDBID_MATCH_ALL)) == CRDBID_MATCH_ALL)
    MatchMethod::All;
#else
    MatchMethod::Any
#endif
  ;

  constexpr bool const kUseCartId32a = (((OPTION_CRDB_MULTIMATCH_CART) & (CRDBID_USE_ID32A)) == (CRDBID_USE_ID32A));
  constexpr bool const kUseCartId32b = (((OPTION_CRDB_MULTIMATCH_CART) & (CRDBID_USE_ID32B)) == (CRDBID_USE_ID32B));

  constexpr bool const kUseDatabaseId32a = (((OPTION_CRDB_MULTIMATCH_DB) & (CRDBID_USE_ID32A)) == (CRDBID_USE_ID32A));
  constexpr bool const kUseDatabaseId32b = (((OPTION_CRDB_MULTIMATCH_DB) & (CRDBID_USE_ID32B)) == (CRDBID_USE_ID32B));

  constexpr bool const kMatchNormal = MATCH_NORMAL;
  constexpr bool const kMatchInverted = MATCH_INVERT;

  constexpr bool const kMatchAtoA = kUseCartId32a && kUseDatabaseId32a && kMatchNormal;
  constexpr bool const kMatchBtoB = kUseCartId32b && kUseDatabaseId32b && kMatchNormal;

  constexpr bool const kMatchAtoB = kUseCartId32a && kUseDatabaseId32b && kMatchInverted;
  constexpr bool const kMatchBtoA = kUseCartId32b && kUseDatabaseId32a && kMatchInverted;

  __attribute__((hot, always_inline))
  static inline bool _matches(crc32_t * search32, crc32_t * id32)
  {
    return ((search32 != nullptr) && (id32 != nullptr) && (*search32 == *id32));
  }

  __attribute__((hot, always_inline))
  static inline bool _matchesEither(crc32_t * search32, crc32_t * id32a, crc32_t * id32b)
  {
    if (search32 == nullptr) return false;
    return (*search32 == *id32a) || (*search32 == *id32b);
  }

  __attribute__((hot, always_inline))
  static inline bool _matchesBoth(crc32_t * search32, crc32_t * id32a, crc32_t * id32b)
  {
    if (search32 == nullptr) return false;
    return (*search32 == *id32a) && (*search32 == *id32b);
  }

  __attribute__((hot, always_inline))
  static inline bool _hasMatch(crc32_t * search32a, crc32_t * search32b, crc32_t * id32a, crc32_t * id32b)
  {
    bool matchedAtoA = false;
    bool matchedAtoB = false;
    bool matchedBtoA = false;
    bool matchedBtoB = false;

    if __if_constexpr (kMatchAtoA)
    {
      if ((search32a != nullptr) && (id32a != nullptr))
      {
        matchedAtoA = _matches(search32a, id32a);
      }
    }

    if __if_constexpr (kMatchAtoB)
    {
      if ((search32a != nullptr) && (id32b != nullptr))
      {
        matchedAtoB = _matches(search32a, id32b);
      }
    }

    if __if_constexpr (kMatchBtoB)
    {
      if ((search32b != nullptr) && (id32b != nullptr))
      {
        matchedBtoB = _matches(search32b, id32b);
      }
    }

    if __if_constexpr (kMatchBtoA)
    {
      if ((search32b != nullptr) && (id32a != nullptr))
      {
        matchedBtoA = _matches(search32b, id32a);
      }
    }

    switch (kMatchMethod)
    {
    case MatchMethod::Any:
      return (matchedAtoA || matchedBtoB || matchedAtoB || matchedBtoA);

    case MatchMethod::All:
      if __if_constexpr (kMatchNormal)
      {
        if (matchedAtoA && matchedBtoB) return true;
      }

      if __if_constexpr (kMatchInverted)
      {
        if (matchedAtoB && matchedBtoA) return true;
      }

      return false;

    case MatchMethod::Either:
      if __if_constexpr (kMatchNormal)
      {
        if (matchedAtoA || matchedBtoB) return true;
      }

      if __if_constexpr (kMatchInverted)
      {
        if (matchedAtoB || matchedBtoA) return true;
      }

      return false;
    }

    return false;
  }

  bool CRDBBase::searchRecord(uint32_t & target, uint16_t startingRecord, bool reverse, uint16_t offset)
  {
    uint32_t data;
    search.startingRecord = startingRecord;
    search.reverse = reverse;

    clearError();

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> Searching..."));

    OSCR::Serial::print(F("<CRDB> Search: "));
    OSCR::Serial::printHexLine(data);
#endif /* CRDB_DEBUGGING */

    for (search.i = startingRecord; gotoRecordIndex(search.i) && file.seekCur(offset);)
    {
      if (readNum32(&data) != 4) break;

      if (data == target)
      {
        loadRecordIndex(search.i);

#if CRDB_DEBUGGING
        OSCR::Serial::print(F("<CRDB> "));
        OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#endif /* CRDB_DEBUGGING */

        return true;
      }

      // Will infinite loop otherwise
      if (!setNextIndex()) return false;
    }

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#endif /* CRDB_DEBUGGING */

    gotoRecordIndex(0);
    return false;
  }

  bool CRDBNamedByCRC32Base::searchEitherRecord(crc32_t & search32a, crc32_t & search32b, uint16_t startingRecord, bool reverse, uint16_t offset)
  {
    return searchEitherRecord(&search32a, &search32b, startingRecord, reverse, offset);
  }

  bool CRDBNamedByCRC32Base::searchEitherRecord(crc32_t * search32a, crc32_t * search32b, uint16_t startingRecord, bool reverse, uint16_t offset)
  {
    crc32_t id32a, id32b;
    search.startingRecord = startingRecord;
    search.reverse = reverse;

    clearError();

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> Searching..."));

    OSCR::Serial::print(F("<CRDB> Search[A]: "));
    OSCR::Serial::printLine(*search32a);

    OSCR::Serial::print(F("<CRDB> Search[B]: "));
    OSCR::Serial::printLine(*search32b);
#endif /* CRDB_DEBUGGING */

    for (search.i = startingRecord; gotoRecordIndex(search.i) && file.seekCur(offset);)
    {
      if (readNum32(id32a) != 4) break;
      if (readNum32(id32b) != 4) break;

      if (_hasMatch(search32a, search32b, &id32a, &id32b))
      {
        loadRecordIndex(search.i);

#if CRDB_DEBUGGING
        OSCR::Serial::print(F("<CRDB> "));
        OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#endif /* CRDB_DEBUGGING */

        return true;
      }

      // Will infinite loop otherwise
      if (!setNextIndex()) return false;
    }

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#endif /* CRDB_DEBUGGING */

    gotoRecordIndex(0);
    return false;
  }


  bool CRDBNamedByCRC32Base::findEitherRecord(crc32_t crc32search1, crc32_t crc32search2, uint16_t offset)
  {
    clearError();

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> Searching..."));

    OSCR::Serial::print(F("<CRDB> CRC32 Search[1]: "));
    OSCR::Serial::printLine(crc32search1);

    OSCR::Serial::print(F("<CRDB> CRC32 Search[2]: "));
    OSCR::Serial::printLine(crc32search2);
#endif /* CRDB_DEBUGGING */

    for (uint_fast32_t i = 0; gotoRecordIndex(i) && file.seekCur(offset); ++i)
    {
#if OPTION_CRDB_STRICT_MATCHING
      crc32_t id32a;
      crc32_t id32b;

      if (readNum32(id32a) != 4) break;
      if (readNum32(id32b) != 4) break;

      if (
        ((id32a == crc32search1) && (id32b == crc32search2)) || // Testing a/1 and b/2
        ((id32b == crc32search1) && (id32a == crc32search2))    // Testing b/1 and a/2
      )
#else
      crc32_t id32;

      if (readNum32(id32) != 4) break;

      if (id32 == crc32search1 || id32 == crc32search2)         // If either match
#endif
      {
        loadRecordIndex(i);

#if CRDB_DEBUGGING
        OSCR::Serial::print(F("<CRDB> "));
        OSCR::Serial::printLine(FS(OSCR::Strings::Common::OK));
#endif /* CRDB_DEBUGGING */

        return true;
      }
    }

#if CRDB_DEBUGGING
    OSCR::Serial::printLine(F("<CRDB> NOT FOUND"));
#endif /* CRDB_DEBUGGING */

    gotoRecordIndex(0);
    return false;
  }

#if defined(NEEDS_CRDB_ISQUIET)
  bool CRDBBase::isQuiet()
  {
    return beQuiet;
  }

  void CRDBBase::quiet(bool shouldBeQuiet = true)
  {
    beQuiet = shouldBeQuiet;
  }
#endif

} /* namespace OSCR::CRDB */
