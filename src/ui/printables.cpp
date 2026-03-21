#include "api/Storage.h"
#include "hardware/outputs/Serial.h"
#include "ui.h"

namespace OSCR
{
  namespace UI
  {

#pragma region UI/Async

    //
    // Path
    //

    void print(OSCR::Storage::Path const * path)
    {
      char pathStr[OSCR::Storage::kFilePathnameLength];

      path->getPath(BUFFN(pathStr));

      print(pathStr);
    }

    void printLine(OSCR::Storage::Path const * path)
    {
      print(path);
      printLine();
    }

    void print(OSCR::Storage::Path const & path)
    {
      print(&path);
    }

    void printLine(OSCR::Storage::Path const & path)
    {
      print(&path);
      printLine();
    }

    //
    // CRC32
    //

    void print(OSCR::CRC32::crc32_t const * crc32)
    {
      char crc32str[9];

      crc32->human(BUFFN(crc32str));

      print(crc32str);
    }

    void printLine(OSCR::CRC32::crc32_t const * crc32)
    {
      print(crc32);
      printLine();
    }

    void print(OSCR::CRC32::crc32_t const & crc32)
    {
      print(&crc32);
    }

    void printLine(OSCR::CRC32::crc32_t const & crc32)
    {
      print(&crc32);
      printLine();
    }

#pragma region UI/Sync

    //
    // Path
    //

    void printSync(OSCR::Storage::Path const * path)
    {
      char pathStr[OSCR::Storage::kFilePathnameLength];

      path->getPath(BUFFN(pathStr));

      print<true>(pathStr);
    }

    void printLineSync(OSCR::Storage::Path const * path)
    {
      OSCR::UI::print(path);
      OSCR::UI::printLine<true>();
    }

    void printSync(OSCR::Storage::Path const & path)
    {
      printSync(&path);
    }

    void printLineSync(OSCR::Storage::Path const & path)
    {
      OSCR::UI::print(&path);
      OSCR::UI::printLine<true>();
    }

    void printValue(char const * label, OSCR::Storage::Path const & path)
    {
      char value[OSCR::Storage::kFilePathnameLength];

      path.getPath(BUFFN(value));

      OSCR::UI::printLineSync(OSCR::Lang::formatLabel(label, value));
    }

    //
    // CRC32
    //

    void printSync(OSCR::CRC32::crc32_t const * crc32)
    {
      char crc32str[9];

      crc32->human(BUFFN(crc32str));

      OSCR::UI::print<true>(crc32str);
    }

    void printLineSync(OSCR::CRC32::crc32_t const * crc32)
    {
      OSCR::UI::print(crc32);
      OSCR::UI::printLine<true>();
    }

    void printSync(OSCR::CRC32::crc32_t const & crc32)
    {
      printSync(&crc32);
    }

    void printLineSync(OSCR::CRC32::crc32_t const & crc32)
    {
      OSCR::UI::print(&crc32);
      OSCR::UI::printLine<true>();
    }

    void printValue(char const * label, OSCR::CRC32::crc32_t const & crc32)
    {
      char value[9];

      crc32.human(BUFFN(value));

      OSCR::UI::printLineSync(OSCR::Lang::formatLabel(label, value));
    }
  }

# if defined(ENABLE_SERIAL_OUTPUT) || defined(ENABLE_UPDATER)

  namespace Serial
  {

#pragma region Serial/Async

    //
    // Path
    //

    void print(OSCR::Storage::Path const * path)
    {
      char pathStr[OSCR::Storage::kFilePathnameLength];

      path->getPath(BUFFN(pathStr));

      print(pathStr);
    }

    void printLine(OSCR::Storage::Path const * path)
    {
      print(path);
      printLine();
    }

    void print(OSCR::Storage::Path const & path)
    {
      print(&path);
    }

    void printLine(OSCR::Storage::Path const & path)
    {
      print(&path);
      printLine();
    }

    //
    // CRC32
    //

    void print(OSCR::CRC32::crc32_t const * crc32)
    {
      char crc32str[9];

      crc32->human(BUFFN(crc32str));

      print(crc32str);
    }

    void printLine(OSCR::CRC32::crc32_t const * crc32)
    {
      print(crc32);
      printLine();
    }

    void print(OSCR::CRC32::crc32_t const & crc32)
    {
      print(&crc32);
    }

    void printLine(OSCR::CRC32::crc32_t const & crc32)
    {
      print(&crc32);
      printLine();
    }


#pragma region Serial/Sync

    //
    // Path
    //

    void printSync(OSCR::Storage::Path const * path)
    {
      print(path);
      flushSync();
    }

    void printLineSync(OSCR::Storage::Path const * path)
    {
      print(path);
      printLine();
      flushSync();
    }

    void printSync(OSCR::Storage::Path const & path)
    {
      printSync(&path);
    }

    void printLineSync(OSCR::Storage::Path const & path)
    {
      printLineSync(&path);
    }

    void printValue(char const * label, OSCR::Storage::Path const & path)
    {
      char value[OSCR::Storage::kFilePathnameLength];

      path.getPath(BUFFN(value));

      printLineSync(OSCR::Lang::formatLabel(label, value));
    }

    //
    // CRC32
    //

    void printSync(OSCR::CRC32::crc32_t const * crc32)
    {
      print(crc32);
      flushSync();
    }

    void printLineSync(OSCR::CRC32::crc32_t const * crc32)
    {
      print(crc32);
      printLine();
      flushSync();
    }

    void printSync(OSCR::CRC32::crc32_t const & crc32)
    {
      printSync(&crc32);
    }

    void printLineSync(OSCR::CRC32::crc32_t const & crc32)
    {
      printLineSync(&crc32);
    }

    void printValue(char const * label, OSCR::CRC32::crc32_t const & crc32)
    {
      char value[9];

      crc32.human(BUFFN(value));

      printLineSync(OSCR::Lang::formatLabel(label, value));
    }
  }

#endif

}
