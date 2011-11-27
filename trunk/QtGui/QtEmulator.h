/* This file is part of 34S.
 *
 * 34S is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 34S is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 34S.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QTEMULATOR_H_
#define QTEMULATOR_H_

#include <QtGui>
#include <QMainWindow>
#include "QtSkin.h"
#include "QtBackgroundImage.h"
#include "QtKeyboard.h"
#include "QtScreen.h"
#include "QtCalculatorThread.h"
#include "QtHeartBeatThread.h"

#define ORGANIZATION_NAME "WP-34s"
#define APPLICATION_NAME "WP34sEmulator"
#define WINDOWS_SETTINGS_GROUP "MainWindow"
#define WINDOWS_POSITION_SETTING "Position"
#define DEFAULT_POSITION_X 50
#define DEFAULT_POSITION_Y 50

#define SKIN_SUFFIX "xskin"
#define MEMORY_FILE_TYPE "memory"
#define MEMORY_DIRECTORY "memory"
#define SKIN_FILE_TYPE "skin"
#define SKIN_DIRECTORY "skins"
#define IMAGE_FILE_TYPE "image"
#define IMAGE_DIRECTORY "images"
#define NON_VOLATILE_MEMORY_FILENAME "wp34s.dat"
#define REGION_FILENAME_PATTERN "wp34s-%1.dat"

#ifdef Q_WS_MAC
#define RESOURCES_DIR "/../resources/"
#endif

#define FLASH_REGION_DEFAULT_VALUE 0xFF

class QtEmulator : public QMainWindow
{
	Q_OBJECT

public:
     QtEmulator();
     ~QtEmulator();

public:
     QtKeyboard& getKeyboard() const;
     QtScreen& getScreen() const;
     void updateScreen();
     // Used by program_flash via QtEmulatorAdapter.c
     char* getRegionPath(int aRegionIndex);
     void resetUserMemory();

protected:
     void closeEvent(QCloseEvent* event);

private:
     void setPaths();
     QString getSkinFilename();
     QtSkin* buildSkin(const QString& aStringFilename);
     void buildComponents(const QtSkin& aSkin);
     void startThreads();
     void loadSettings();
     void saveSettings();
     void loadMemory();
     bool loadMemoryRegion(int aRegionIndex);
     void saveMemory();
     QString getRegionName(int aRegionIndex) const;
     QString getRegionFileName(int aRegionIndex) const;
     QString getMemoryPath() const;
     void memoryWarning(const QString& aMessage, bool aResetFlag=true);

private:
     QtKeyboard* keyboard;
     QtScreen* screen;
     QtBackgroundImage* backgroundImage;
     QtCalculatorThread* calculatorThread;
     QtHeartBeatThread* heartBeatThread;
     QSettings settings;
     QString userSettingsDirectoryName;
     // We need to keep this variable to return a properly allocated char*
     // to program_flash in storage.c
     QByteArray currentRegionPath;

signals:
	void screenChanged();
};

extern QtEmulator* currentEmulator;

#endif /* QTEMULATOR_H_ */
