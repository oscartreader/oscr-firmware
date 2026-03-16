#include "apps/FileBrowser.h"

namespace
{
  using OSCR::Apps::FileBrowser;
  using OSCR::BrowserFile;

  void wrapOpenFilePlain(FileBrowser & browser, OSCR::Storage::File & file, oflag_t oflag)
  {
    if (file.isOpen()) file.close();
    if (!browser.openFile(browser.select(), file, oflag)) OSCR::UI::fatalErrorStorage();
  }

  void wrapOpenFileShared(FileBrowser & browser, OSCR::Storage::File & file, oflag_t oflag)
  {
    wrapOpenFilePlain(browser, file, oflag);

    if (file.getName(BUFFN(OSCR::Storage::Shared::sharedFileName)) < 1) OSCR::UI::fatalErrorNameOverflow();
    if (!browser.getPath(BUFFN(OSCR::Storage::Shared::sharedFilePathname))) OSCR::UI::fatalErrorNameOverflow();
  }

  void wrapOpenFile(FileBrowser & browser, OSCR::Storage::File & file, oflag_t oflag, char * nameBuff = nullptr, size_t nameSize = 0, char * pathBuff = nullptr, size_t pathSize = 0)
  {
    wrapOpenFilePlain(browser, file, oflag);

    // = File Name
    if (nameBuff == nullptr) // Is `nameBuff` a `nullptr`?
    {
      // If `nameSize` is 0, that means copy the name into the shared buffer (`OSCR::Storage::Shared::sharedFileName`).
      if (nameSize < 1)
      {
        if(file.getName(BUFFN(OSCR::Storage::Shared::sharedFileName)) < 1) OSCR::UI::fatalErrorNameOverflow();
      }
    }
    else // If `pathBuff` is not a `nullptr`...
    {
      if (nameSize > 1)
      {
        if(file.getName(nameBuff, nameSize) < 1) OSCR::UI::fatalErrorNameOverflow();
      }
    }

    // = File Path
    if (pathBuff == nullptr) // Is `pathBuff` a `nullptr`?
    {
      // If `pathSize` is 0, that means copy the path into the shared buffer (`OSCR::Storage::Shared::sharedFilePathname`).
      if (pathSize < 1)
      {
        if(!browser.getPath(BUFFN(OSCR::Storage::Shared::sharedFilePathname))) OSCR::UI::fatalErrorNameOverflow();
      }
    }
    else // If `pathBuff` is not a `nullptr`...
    {
      // If `pathSize` is larger than 1, copy the path into `pathBuff` and `pathSize`...
      if (pathSize > 1)
      {
        if(!browser.getPath(pathBuff, pathSize)) OSCR::UI::fatalErrorNameOverflow();
      }
    }
  }

  /**
   * @cond
   * Compare files.
   */
  int compareFiles(void const * _a, void const * _b)
  {
    bool aDir = (*(BrowserFile*)_a).isDir;
    bool bDir = (*(BrowserFile*)_b).isDir;
    if (!aDir && bDir) return 1;
    if (aDir && !bDir) return -1;

    for (uint8_t i = 0; i < UI_FILE_BROWSER_FILE_SORT_LEN; ++i)
    {
      uint8_t a = (*(BrowserFile*)_a).name[i];
      uint8_t b = (*(BrowserFile*)_b).name[i];

      if (!a && !b) return 0;
      else if (!a || !b) return (!a ? -1 : 1);

      if (a > 0x40 && a < 0x91) a += 0x20;
      if (b > 0x40 && b < 0x91) b += 0x20;

      if (a != b) return (a > b ? 1 : -1);
    }

    return 0;
  }
  //! @endcond
} /* namespace {anon} */

namespace OSCR
{
  namespace Apps
  {
    FileBrowser::FileBrowser(__FlashStringHelper const * browserTitle)
      : MenuRenderer(browserTitle)
    {
      chdir();

      setup();
    }

    void FileBrowser::setup()
    {
      onPageChange();
    }

    void FileBrowser::onPageChange()
    {
      uint8_t indexOffset = getPageEntryOffset();

      for (uint_fast8_t menuIndex = 0; menuIndex < getPageEntryCount(); ++menuIndex)
      {
        uint8_t entryIndex = indexOffset + menuIndex;

        strlcpy(pageEntries[menuIndex], files[entryIndex].name, UI_PAGE_ENTRY_LENTH_MAX);
      }
    }

    void FileBrowser::chdir()
    {
      cwdString = cwd.getPath();

      if (!dir.open(cwdString.c_str()))
      {
        OSCR::UI::fatalErrorStorage();
      }

      cwdRefresh();
    }

    void FileBrowser::chdir(uint8_t entryIndex)
    {
      cwd.cd(files[entryIndex].name);

      cwdString = cwd.getPath();

      if (!dir.open(cwdString.c_str()))
      {
        OSCR::UI::fatalErrorStorage();
      }

      cwdRefresh();
    }

    void FileBrowser::cwdRefresh()
    {
      count = 0;

      // Count files in directory
      while (file.openNext(&dir, O_READ) && (count < UI_FILE_BROWSER_FILES_MAX))
      {
        if (!file.isHidden() && (file.isDir() || file.isFile()))
        {
          files[count].index = dir.curPosition()/32;
          file.getName(files[count].name, UI_FILE_BROWSER_FILENAME_MAX + 1);
          files[count].isDir = file.isDir();
          count++;
        }

        file.close();
      }

      sort();

      totalPages = max(1, (count / UI_PAGE_SIZE + (count%UI_PAGE_SIZE != 0)));
      pageEntriesLast = count - ((totalPages - 1) * UI_PAGE_SIZE);

      selection = 0;
      gotoPage(1);

      render();
    }

    void FileBrowser::sort()
    {
#if UI_FILE_BROWSER_FILE_SORT_LEN > 0
      qsort(files, count, sizeof(BrowserFile), compareFiles);
#endif /* UI_FILE_BROWSER_FILE_SORT_LEN > 0 */
    }

    bool FileBrowser::openFile(uint8_t entryIndex, OSCR::Storage::File & selectedFile, oflag_t oflag)
    {
      uint32_t index = files[entryIndex].index;
      return selectedFile.open(index, oflag);
    }

    bool FileBrowser::onConfirm()
    {
      if (!files[getEntryIndex()].isDir)
      {
        return true;
      }

      uint8_t idx = getEntryIndex();

      chdir(idx);

      return false;
    }

    bool FileBrowser::getPath(char buffer[], size_t bufferSize)
    {
      return cwd.getPath(buffer, bufferSize);
    }

    String FileBrowser::getPath()
    {
      return cwd.getPath();
    }
  } /* namespace Apps */

  void selectFile(__FlashStringHelper const * title, oflag_t oflag)
  {
    FileBrowser browser(title);

    if (OSCR::Storage::Shared::sharedFile.isOpen()) OSCR::Storage::Shared::sharedFile.close();

    return wrapOpenFileShared(browser, OSCR::Storage::Shared::sharedFile, oflag);
  }

  void selectFile(__FlashStringHelper const * title, OSCR::Storage::File & file, oflag_t oflag, bool useShared)
  {
    FileBrowser browser(title);

    return useShared ? wrapOpenFileShared(browser, file, oflag) : wrapOpenFilePlain(browser, file, oflag);
  }

  void selectFile(__FlashStringHelper const * title, OSCR::Storage::File & file, oflag_t oflag, char * nameBuff, size_t nameSize, char * pathBuff, size_t pathSize)
  {
    FileBrowser browser(title);

    return wrapOpenFile(browser, file, oflag, nameBuff, nameSize, pathBuff, pathSize);
  }

  void selectFile(__FlashStringHelper const * title, __FlashStringHelper const * path, OSCR::Storage::File & file, oflag_t oflag, bool useShared)
  {
    FileBrowser browser(title);

    if (useShared) wrapOpenFileShared(browser, file, oflag);
    else wrapOpenFilePlain(browser, file, oflag);
  }

  void selectFile(__FlashStringHelper const * title, __FlashStringHelper const * path, OSCR::Storage::File & file, oflag_t oflag, char * nameBuff, size_t nameSize, char * pathBuff, size_t pathSize)
  {
    FileBrowser browser(title);

    wrapOpenFile(browser, file, oflag, nameBuff, nameSize, pathBuff, pathSize);
  }
} /* namespace OSCR */
