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
#include <QLabel>
#include "QtSkin.h"
#include "QtBackgroundImage.h"
#include "QtKeyboard.h"
#include "QtScreen.h"
#include "QtCalculatorThread.h"
#include "QtHeartBeatThread.h"
#include "QtSerialPort.h"
#include "QtDebugger.h"

#define ORGANIZATION_NAME "WP-34s"
#define APPLICATION_NAME "WP34sEmulator"
#define WINDOWS_SETTINGS_GROUP "MainWindow"
#define WINDOWS_POSITION_SETTING "Position"
#define WINDOWS_TITLEBAR_VISIBLE_SETTING "Frameless"
#define DEBUGGER_VISIBLE_SETTING "Debugger"
#define DEFAULT_POSITION_X 50
#define DEFAULT_POSITION_Y 50

#define THREAD_WAITING_TIME (HEARTBEART_SLEEP_TIME_IN_MILLISECONDS*2)

#define SKIN_SETTINGS_GROUP "Skin"
#define LAST_SKIN_SETTING "LastSkin"

#define CUSTOM_DIRECTORY_SETTINGS_GROUP "CustomDirectory"
#define CUSTOM_DIRECTORY_ACTIVE_SETTING "CustomDirectoryActive"
#define CUSTOM_DIRECTORY_NAME_SETTING "CustomDirectoryName"
#define LAST_MEMORYFILE_SETTINGS_GROUP "LastMemoryFile"
#define LAST_MEMORYFILE_ACTIVE_SETTING "LastMemoryFile"
#define LAST_MEMORYFILE_SETTING "LastMemoryFileName"

#define TOOLS_SETTINGS_GROUP "Tools"
#define TOOLS_ACTIVE_SETTING "ToolsActive"
#define TOOLS_NAME_SETTING "ToolsName"

#define KEYBOARD_SETTINGS_GROUP "Keyboard"
#define USE_HSHIFT_CLICK_SETTING "UseHShiftClick"
#define ALWAYS_USE_HSHIFT_CLICK_SETTING "AlwaysUseHShiftClick"
#define HSHIFT_DELAY_SETTING "HShiftDelay"
#define SHOW_TOOLTIPS_SETTING "ShowToolTips"
#define DEFAULT_SHOW_TOOLTIPS_SETTING true

#define DISPLAY_SETTINGS_GROUP "Display"
#define USE_FONTS_SETTING "UseFonts"
#define DEFAULT_USE_FONTS_SETTING true
#define SHOW_CATALOG_MENUS_SETTING "ShowCatalogMenus"
#define DEFAULT_SHOW_CATALOG_MENUS_SETTING true
#define CLOSE_CATALOG_MENUS_SETTING "CloseCatalogMenus"
#define DEFAULT_CLOSE_CATALOG_MENUS_SETTING true
#define DISPLAY_AS_STACK_SETTING "DisplayAsStack"


#define SERIAL_PORT_SETTINGS_GROUP "SerialPort"
#define SERIAL_PORT_NAME_SETTING "SerialPortName"

#define SKIN_SUFFIX "xskin"
#define MEMORY_FILE_TYPE "memory"
#define MEMORY_DIRECTORY "memory"
#define SKIN_FILE_TYPE "skin"
#define SKIN_DIRECTORY "skins"
#define IMAGE_FILE_TYPE "image"
#define IMAGE_DIRECTORY "images"
#define DOCUMENTATION_FILE_TYPE "doc"
#define DOCUMENTATION_DIRECTORY "doc"
#define STATE_FILENAME "wp34s.dat"
#define BACKUP_FILENAME "wp34s-backup.dat"
#define LIBRARY_FILENAME "wp34s-lib.dat"
#define FONT_FILE_TYPE "font"
#define FONTS_DIRECTORY "fonts"

#define DOCUMENTATION_FILENAME "Manual_wp_34s_3_1.pdf"
#define WEBSITE_URL "http://wp34s.sourceforge.net/"

#define DEBUG_OPTION "-debug"
#define DEVELOPMENT_OPTION "-dev"

#ifdef Q_WS_MAC
#define RESOURCES_DIR "/../Resources/"
#else
#ifdef Q_OS_UNIX
#define RESOURCES_DIR "/../lib/WP-34s/"
#endif
#endif

#define FLASH_REGION_DEFAULT_VALUE 0xFF

#define MAIN_MENU "Calculator"
#define EDIT_MENU "Edit"
#define DEBUG_MENU "Debug"
#define SKINS_MENU "Skins"
#define HELP_MENU "Help"

#define ABOUT_ACTION_TEXT "About"
#define PREFERENCES_ACTION_TEXT "Preferences"

#define HIDE_TITLEBAR_ACTION_TEXT "Hide TitleBar"
#define SHOW_TITLEBAR_ACTION_TEXT "Show TitleBar"
#define RESET_ACTION_TEXT "Reset Memory"
#define RELOAD_ACTION_TEXT "Reload State"
#define OPEN_ACTION_TEXT "Load State..."
#define SAVE_ACTION_TEXT "Save State"
#define SAVE_AS_ACTION_TEXT "Save State As..."
#define IMPORT_ACTION_TEXT "Import Program..."
#define EXPORT_ACTION_TEXT "Export Program..."
#define USE_DEFAULT_MEMORY_TEXT "Use Default State"

#define QUIT_ACTION_TEXT "Quit"

#define COPY_NUMBER_ACTION_TEXT "Copy Number"
#define COPY_RAW_X_ACTION_TEXT "Copy Raw"
#define COPY_TEXTLINE_ACTION_TEXT "Copy Textline"
#define COPY_IMAGE_ACTION_TEXT "Copy Screen Image"
#define PASTE_NUMBER_ACTION_TEXT "Paste Number"
#define PASTE_RAW_X_ACTION_TEXT "Paste Raw"

#define HIDE_DEBUGGER_ACTION_TEXT "Hide Debugger"
#define SHOW_DEBUGGER_ACTION_TEXT "Show Debugger"


#define SHOW_WEBSITE_ACTION_TEXT "Show Website"
// Manual could be change to "Documentation" if we can generate it in HTML for instance
#define SHOW_DOCUMENTATION_ACTION_TEXT "Manual"

#define STATE_FILE_FILTER "State Files (*.dat *.*)"
#define IMPORT_FILE_FILTER "Text Files (*.wp34s *.txt)"
#define EXPORT_FILE_FILTER "Text Files (*.wp34s)"

typedef QMap<QString, QString> SkinMap;

class QtEmulator: public QMainWindow
{
	Q_OBJECT

public:
     QtEmulator();
     ~QtEmulator();

public:
     void setVisible(bool visible);
     void quit();
     QtKeyboard& getKeyboard() const;
     QtScreen& getScreen() const;
     QtSerialPort& getSerialPort() const;
     QtDebugger& getDebugger() const;
     void updateScreen();
     // Used by program_flash via QtEmulatorAdapter.c
     char* getRegionPath(int aRegionIndex);
     void resetUserMemory();
     void onCatalogStateChanged();
     void showCatalogMenu();
     void showMessage(const char* title, const char* message);

private slots:
	void editPreferences();
	void showAbout();
	void toggleTitleBar();
	void setTitleBarVisible(bool aTitleBarVisibleFlag);
	void confirmReset();
	void copyNumber();
	void copyRawX();
	void copyTextLine();
	void copyImage();
	void pasteNumber();
	void pasteRawX();
    void toggleDebugger();
	void selectSkin(QAction* anAction);
	void showWebSite();
	void showDocumentation();
	void showContextMenu(const QPoint& aPoint);
	void reload();
	void open();
	void save();
	void saveAs();
	void importState();
	void exportState();
	void useDefaultState();

private:
     void setPaths();
     void checkCustomDirectory();
     void checkToolsDirectory();
     void forwardToolsSettings();
     void buildMenus();
     void buildContextMenu();
     void buildMainMenu();
     void buildEditMenu();
     void buildDebugMenu();
     void buildSkinsMenu();
     void buildHelpMenu();
     void buildContextualQuit();
     void buildComponents(const QtSkin& aSkin);
     void buildSerialPort();
     void buildDebugger();
     void startThreads();
     void stopThreads();
     void loadSettings();
     void loadUserInterfaceSettings();
     void loadKeyboardSettings();
     void loadDisplaySettings();
     void loadCustomDirectorySettings();
     void loadLastMemoryFileSettings();
     void loadSerialPortSettings();
     void loadToolsSettings();
     void saveSettings();
     void saveUserInterfaceSettings();
     void saveKeyboardSettings();
     void saveDisplaySettings();
     void saveCustomDirectorySettings();
     void saveLastMemoryFileSettings();
     void saveSerialPortSettings();
     void saveToolsSettings();
     void loadMemory();
     void loadState();
     void loadBackup();
     void loadLibrary();
     void saveMemory();
     void saveState();
     void saveBackup();
     void saveLibrary();
     QString getMemoryPath(const QString& aMemoryFilename) const;
     QString getRegionFileName(int aRegionIndex) const;
     void memoryWarning(const QString& aMessage, bool aResetFlag=true);
     QtSkin* buildSkin(const QString& aStringFilename) throw (QtSkinException);
     void setInitialSkin() throw (QtSkinException);
     void findSkins();
     void setSkin(const QString& aSkinName) throw (QtSkinException);
     void skinError(const QString& aMessage, bool aFatalFlag);
     void setTransparency(bool enabled);
     void setDebuggerVisible(bool aDebuggerVisible);
     void setLastMemoryFile(QString& aFilename);

private:
     QtKeyboard* keyboard;
     QtScreen* screen;
     QWidget* centralWidget;
     QLayout* centralLayout;
     QtBackgroundImage* backgroundImage;
     QtCalculatorThread* calculatorThread;
     QtHeartBeatThread* heartBeatThread;
     QtDebugger* debugger;
     QSettings settings;
     QString userSettingsDirectoryName;
     // We need to keep this variable to return a properly allocated char*
     // to program_flash in storage.c
     QByteArray currentRegionPath;
     bool customDirectoryActive;
     QDir customDirectory;
     bool lastMemoryFileActive;
     QString lastMemoryFile;
     QString currentMemoryFile;
     bool toolsActive;
     QDir tools;
     QActionGroup* skinsActionGroup;
     SkinMap skins;
     QString currentSkinName;
     bool useHShiftClick;
     bool alwaysUseHShiftClick;
     int hShiftDelay;
     bool showToolTips;
     bool useFonts;
     bool showCatalogMenus;
     bool closeCatalogMenus;
     bool displayAsStack;
     QtSerialPort* serialPort;
     bool active;
     bool development;
     bool debug;
     bool titleBarVisible;
     bool debuggerVisible;
     QAction* toggleTitleBarAction;
     QAction* toggleDebuggerAction;
     QMenu* contextMenu;
     Qt::WindowFlags titleBarVisibleFlags;
     QVector<QAction*> skinActions;

signals:
	void screenChanged();
	void catalogStateChanged();
};

extern QtEmulator* currentEmulator;

#endif /* QTEMULATOR_H_ */
