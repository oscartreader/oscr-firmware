/**
 * @file
 * @brief OSCR File Browser application
 */
#pragma once
#ifndef OSCR_FILE_BROWSER_H_
# define OSCR_FILE_BROWSER_H_

#include "common/OSCR.h"
#include "common/Util.h"
#include "ui.h"

namespace OSCR
{
  namespace Apps
  {
    /**
     * @class FileBrowser
     *
     * @brief An interface for handling choosing a file.
     *
     * Create a file browser interface.
     *
     * @note
     * Available memory on embedded microcontrollers is limited. You
     * should consider using OSCR::UI::menu() instead of implementing
     * this class directly.
     */
    class FileBrowser: public OSCR::UI::MenuRenderer
    {
    public:
      /**
       * Create a file browser interface for a path.
       *
       * @param browserTitle  A flash string to use for the menu title.
       *
       * @todo Actually use the `path` parameter.
       */
      FileBrowser(__FlashStringHelper const * browserTitle);

      /**
       * Create a file browser interface for a path.
       *
       * @param browserTitle  A flash string to use for the menu title.
       * @param path          Directory path to start in.
       *
       * @todo Actually use the `path` parameter.
       */
      FileBrowser(__FlashStringHelper const * browserTitle, __FlashStringHelper const * path);

      /**
       * Get the file represented by a specific index.
       *
       * @param   entryIndex    Index of the desired file.
       * @param   selectedFile  Variable to store the file in.
       */
      bool openFile(uint8_t entryIndex, OSCR::Storage::File & selectedFile, oflag_t oflag);

      bool getPath(char buffer[], size_t bufferSize = OSCR::Storage::kMaxPathLength);
      String getPath();

    protected:
      //! @internal
      FsFile dir;
      FsFile file;
      BrowserFile files[UI_FILE_BROWSER_FILES_MAX];

      OSCR::Storage::Path cwd = OSCR::Storage::Path();
      String cwdString = cwd.getPath();

      void setup();
      void onPageChange();
      void chdir();
      void chdir(uint8_t entryIndex);
      void cwdRefresh();
      void sort();
      bool onConfirm();
    };
  }

  extern void selectFile(__FlashStringHelper const * title, oflag_t oflag);
  extern void selectFile(__FlashStringHelper const * title, OSCR::Storage::File & file, oflag_t oflag, bool useShared = true);
  extern void selectFile(__FlashStringHelper const * title, OSCR::Storage::File & file, oflag_t oflag, char * nameBuff, size_t nameSize, char * pathBuff = nullptr, size_t pathSize = 0);
  extern void selectFile(__FlashStringHelper const * title, __FlashStringHelper const * path, OSCR::Storage::File & file, oflag_t oflag, bool useShared = true);
  extern void selectFile(__FlashStringHelper const * title, __FlashStringHelper const * path, OSCR::Storage::File & file, oflag_t oflag, char * nameBuff, size_t nameSize, char * pathBuff = nullptr, size_t pathSize = 0);
}

#endif /* OSCR_FILE_BROWSER_H_ */
