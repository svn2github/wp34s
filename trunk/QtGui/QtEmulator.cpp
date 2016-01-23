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

#include <string.h>
#include "QtEmulator.h"
#include "QtEmulatorAdapter.h"
#include "QtBuildDate.h"
#include "QtPreferencesDialog.h"
#include "QtNumberPaster.h"

QtEmulator* currentEmulator;

QtEmulator::QtEmulator()
: calculatorThread(NULL), heartBeatThread(NULL), debugger(NULL), skinsActionGroup(NULL), titleBarVisible(true), debuggerVisible(false)
{
	debug=qApp->arguments().contains(DEBUG_OPTION);
	development=qApp->arguments().contains(DEVELOPMENT_OPTION);

#ifdef Q_WS_MAC
	QSettings::Format format=QSettings::NativeFormat;
#else
	QSettings::Format format=QSettings::IniFormat;
#endif
	QSettings userSettings(format, QSettings::UserScope, QString(ORGANIZATION_NAME), QString(APPLICATION_NAME));
	userSettingsDirectoryName=QFileInfo(userSettings.fileName()).dir().path();

#ifdef Q_WS_MAC
	userSettingsDirectoryName.append('/').append(ORGANIZATION_NAME);
#endif
	QDir userSettingsDirectory(userSettingsDirectoryName);
	if(!userSettingsDirectory.exists())
	{
		userSettingsDirectory.mkpath(userSettingsDirectoryName);
	}

	buildSerialPort();
	loadSettings();
	setPaths();

	// setInitialSkin must be before buildSkinMenu, called from buildMenuBar
	setInitialSkin();
	buildMenus();

	connect(this, SIGNAL(screenChanged()), backgroundImage, SLOT(updateScreen()));

	setWindowTitle(QApplication::translate("wp34s", "WP34s"));
	currentEmulator = this;

	loadMemory();
	buildDebugger();
	startThreads();
	setTitleBarVisible(titleBarVisible);
	setDebuggerVisible(debuggerVisible);
	active=true;
}

QtEmulator::~QtEmulator()
{
	if(active)
	{
		quit();
	}
}

void QtEmulator::setVisible(bool visible)
{
	if(visible)
	{
	  	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
		if(titleBarVisible)
		{
			titleBarVisibleFlags=windowFlags();
		}

		setFixedSize(sizeHint());
	}

    QMainWindow::setVisible(visible);
}

void QtEmulator::quit()
{
	saveSettings();
	saveMemory();

	delete skinsActionGroup;
	stopThreads();
	delete heartBeatThread;
	delete calculatorThread;
	active=false;
}

QtKeyboard& QtEmulator::getKeyboard() const
{
	return *keyboard;
}

QtScreen& QtEmulator::getScreen() const
{
	return *screen;
}

QtSerialPort& QtEmulator::getSerialPort() const
{
	return *serialPort;
}

QtDebugger& QtEmulator::getDebugger() const
{
	return *debugger;
}

void QtEmulator::updateScreen()
{
	emit screenChanged();
}

void QtEmulator::editPreferences()
{
	QtPreferencesDialog preferencesDialog(customDirectoryActive,
			customDirectory.path(),
			keyboard->isUseHShiftClick(),
			keyboard->isAlwaysUseHShiftClick(),
			keyboard->getHShiftDelay(),
			keyboard->isShowToolTips(),
			screen->isUseFonts(),
			backgroundImage->isShowCatalogMenu(),
			backgroundImage->isCloseCatalogMenu(),
			debugger->isDisplayAsStack(),
			serialPort->getSerialPortName(),
			toolsActive,
			tools.path(),
			currentMemoryFile,
			this);
	int result=preferencesDialog.exec();
	if(result==QDialog::Accepted)
	{
		customDirectoryActive=preferencesDialog.isCustomDirectoryActive();
		customDirectory.setPath(preferencesDialog.getCustomDirectoryName());
		checkCustomDirectory();
		saveCustomDirectorySettings();

		keyboard->setUseHShiftClick(preferencesDialog.isUseHShiftClickActive());
		keyboard->setAlwaysUseHShiftClick(preferencesDialog.isAlwaysUseHShiftClickActive());
		keyboard->setHShiftDelay(preferencesDialog.getHShiftDelay());
		keyboard->setShowToolTips(preferencesDialog.isShowToolTips());
		saveKeyboardSettings();

		backgroundImage->showToolTips(preferencesDialog.isShowToolTips());

		screen->setUseFonts(preferencesDialog.isUseFonts());
		backgroundImage->setShowCatalogMenu(preferencesDialog.isShowCatalogMenus());
		backgroundImage->setCloseCatalogMenu(preferencesDialog.isSCloseCatalogMenus());
		debugger->setDisplayAsStack(preferencesDialog.isDisplayAsStack());
		saveDisplaySettings();

		QString serialPortName=preferencesDialog.getSerialPortName();
		serialPort->setSerialPortName(serialPortName);
		saveSerialPortSettings();

		toolsActive=preferencesDialog.isToolsActive();
		tools.setPath(preferencesDialog.getToolsName());
		checkToolsDirectory();
		saveToolsSettings();
		forwardToolsSettings();

		settings.sync();
		setPaths();
	}
}

void QtEmulator::checkCustomDirectory()
{
	if(customDirectoryActive && (!customDirectory.exists() || !customDirectory.isReadable()))
	{
		customDirectoryActive=false;
		memoryWarning("Cannot find or read custom directory "+customDirectory.path(), false);
	}
}

void QtEmulator::checkToolsDirectory()
{
	if(toolsActive && (!tools.exists() || !tools.isReadable()))
	{
		toolsActive=false;
		memoryWarning("Cannot find or read tools directory "+tools.path(), false);
	}
}


void QtEmulator::showAbout()
{
	QString aboutMessage("WP-34s scientific calculator ");
	aboutMessage+=get_version_string();
	aboutMessage+="\nby Pauli, Walter & Marcus";
	aboutMessage+="\nParts (c) 2008 Hewlett-Packard development L.L.P";
	aboutMessage+="\nGUI by Pascal";
	aboutMessage+=QString("\nBuild date: ")+get_build_date();
	aboutMessage+=QString("\nSvn revision: ")+get_svn_revision_string();

	QMessageBox::about(this, "About WP-34s", aboutMessage);
}

void QtEmulator::showWebSite()
{
	QDesktopServices::openUrl(QUrl(WEBSITE_URL, QUrl::TolerantMode));
}

void QtEmulator::showDocumentation()
{
	QFile documentationFile(QString(DOCUMENTATION_FILE_TYPE)+':'+DOCUMENTATION_FILENAME);
	if(documentationFile.exists())
	{
		QUrl url=QUrl::fromLocalFile(documentationFile.fileName());
		QDesktopServices::openUrl(url);
	}
}

void QtEmulator::toggleTitleBar()
{
	setTitleBarVisible(!titleBarVisible);
}

void QtEmulator::setTitleBarVisible(bool aTitleBarVisibleFlag)
{
	titleBarVisible=aTitleBarVisibleFlag;
	if(titleBarVisible)
	{
		setWindowFlags(titleBarVisibleFlags);
		menuBar()->show();
		setTransparency(false);
		toggleTitleBarAction->setText(HIDE_TITLEBAR_ACTION_TEXT);
	}
	else
	{
#ifdef Q_WS_WIN
		setWindowFlags(Qt::FramelessWindowHint);
#else
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#endif
		menuBar()->hide();
		setTransparency(true);
		toggleTitleBarAction->setText(SHOW_TITLEBAR_ACTION_TEXT);
	}
	show();
}

void QtEmulator::setTransparency(bool enabled)
{
	    if (enabled)
	    {
	        setAutoFillBackground(false);
	    }
	    else
	    {
	        setAttribute(Qt::WA_NoSystemBackground, false);
	    }

	    setAttribute(Qt::WA_TranslucentBackground, enabled);
	    repaint();

}

void QtEmulator::confirmReset()
{
	QMessageBox::StandardButton answer=QMessageBox::question(this,
			"Confirm Memory Reset",
			"Do you really want to reset? This will clear your non-volatile and your flash memory",
			QMessageBox::Ok | QMessageBox::Cancel,
			QMessageBox::Cancel);
	if(answer==QMessageBox::Ok)
	{
		resetUserMemory();
	}
}

void QtEmulator::copyNumber()
{
	qApp->clipboard()->setText(get_formatted_displayed_number());
}

void QtEmulator::copyTextLine()
{
	int* rawText=get_displayed_text();
	int length=0;
	while(rawText[length]!=0)
	{
		length++;
	}
	QChar* text=new QChar[length];
	for(int i=0; i<length; i++)
	{
		text[i]=QChar(rawText[i]);
	}
	qApp->clipboard()->setText(QString(text, length));
}

void QtEmulator::copyImage()
{
	screen->copy(*backgroundImage, *qApp->clipboard());
}

void QtEmulator::pasteNumber()
{
	QtNumberPaster::paste(qApp->clipboard()->text(), *keyboard);
}

void QtEmulator::copyRawX()
{
	char buffer[66];
	char* p = fill_buffer_from_raw_x(buffer);
	if(p!=NULL)
	{
		qApp->clipboard()->setText(QString(p));
	}
}

void QtEmulator::pasteRawX()
{
	paste_raw_x(qApp->clipboard()->text().toStdString().c_str());
}

void QtEmulator::selectSkin(QAction* anAction)
{
	QString savedSkinName=currentSkinName;
	try
	{
		setSkin(anAction->text());
	}
	catch(QtSkinException& exception)
	{
		try
		{
			skinError(exception.what(), false);
			setSkin(savedSkinName);
			for(QVector<QAction*>::const_iterator i=skinActions.constBegin(); i!=skinActions.constEnd(); ++i)
			{
				QAction* action = *i;
				action->setChecked(action->text()==currentSkinName);
			}
		}
		catch(QtSkinException& exception)
		{
			skinError(exception.what(), true);
		}
	}
}

void QtEmulator::buildMenus()
{
	buildContextMenu();
	buildMainMenu();
	buildEditMenu();
	buildDebugMenu();
	buildSkinsMenu();
	buildHelpMenu();
    buildContextualQuit();
}

void QtEmulator::buildContextMenu()
{
	contextMenu=new QMenu(this);
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
}

void QtEmulator::buildMainMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* mainMenu=new QMenu(MAIN_MENU);
	menuBar->addMenu(mainMenu);
	QMenu* mainContextMenu=contextMenu->addMenu(MAIN_MENU);

	toggleTitleBarAction=mainMenu->addAction(HIDE_TITLEBAR_ACTION_TEXT, this, SLOT(toggleTitleBar()));
	mainContextMenu->addAction(toggleTitleBarAction);

	QAction* resetAction=mainMenu->addAction(RESET_ACTION_TEXT, this, SLOT(confirmReset()));
	mainContextMenu->addAction(resetAction);
	mainContextMenu->addSeparator();

	mainMenu->addSeparator();

	QAction* reloadAction=mainMenu->addAction(RELOAD_ACTION_TEXT, this, SLOT(reload()));
	mainContextMenu->addAction(reloadAction);
	QAction* openAction=mainMenu->addAction(OPEN_ACTION_TEXT, this, SLOT(open()));
	mainContextMenu->addAction(openAction);
	QAction* saveAction=mainMenu->addAction(SAVE_ACTION_TEXT, this, SLOT(save()));
	mainContextMenu->addAction(saveAction);
	QAction* saveAsAction=mainMenu->addAction(SAVE_AS_ACTION_TEXT, this, SLOT(saveAs()));
	mainContextMenu->addAction(saveAsAction);

	QAction* useDefaultAction=mainMenu->addAction(USE_DEFAULT_MEMORY_TEXT, this, SLOT(useDefaultState()));
	mainContextMenu->addAction(useDefaultAction);

	mainMenu->addSeparator();
	mainContextMenu->addSeparator();

	QAction* importAction=mainMenu->addAction(IMPORT_ACTION_TEXT, this, SLOT(importState()));
	mainContextMenu->addAction(importAction);
	QAction* exportAction=mainMenu->addAction(EXPORT_ACTION_TEXT, this, SLOT(exportState()));
	mainContextMenu->addAction(exportAction);

	mainContextMenu->addSeparator();


#ifndef Q_WS_MAC
	mainMenu->addSeparator();
	mainMenu->addAction(QUIT_ACTION_TEXT, qApp, SLOT(quit()), QKeySequence::Quit);
#endif
}

void QtEmulator::buildEditMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* editMenu=new QMenu(EDIT_MENU);
	menuBar->addMenu(editMenu);
	QMenu* editContextMenu=contextMenu->addMenu(EDIT_MENU);

	QAction* copyNumberAction=editMenu->addAction(COPY_NUMBER_ACTION_TEXT, this, SLOT(copyNumber()), QKeySequence::Copy);
	editContextMenu->addAction(copyNumberAction);
	QAction* copyRawXAction=editMenu->addAction(COPY_RAW_X_ACTION_TEXT, this, SLOT(copyRawX()));
	editContextMenu->addAction(copyRawXAction);
	QAction* copyTextLineAction=editMenu->addAction(COPY_TEXTLINE_ACTION_TEXT, this, SLOT(copyTextLine()));
	editContextMenu->addAction(copyTextLineAction);
	QAction* copyImageAction=editMenu->addAction(COPY_IMAGE_ACTION_TEXT, this, SLOT(copyImage()));
	editContextMenu->addAction(copyImageAction);
	QAction* pasteNumberAction=editMenu->addAction(PASTE_NUMBER_ACTION_TEXT, this, SLOT(pasteNumber()), QKeySequence::Paste);
	editContextMenu->addAction(pasteNumberAction);
	QAction* pasteRawXAction=editMenu->addAction(PASTE_RAW_X_ACTION_TEXT, this, SLOT(pasteRawX()));
	editContextMenu->addAction(pasteRawXAction);

#ifndef Q_WS_MAC
	editMenu->addSeparator();
#endif
	editContextMenu->addSeparator();
	QAction* preferencesAction=editMenu->addAction(PREFERENCES_ACTION_TEXT, this, SLOT(editPreferences()));
	preferencesAction->setMenuRole(QAction::PreferencesRole);
	editContextMenu->addAction(preferencesAction);
}

void QtEmulator::buildDebugMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* debugMenu=new QMenu(DEBUG_MENU);
	menuBar->addMenu(debugMenu);
	QMenu* debugContextMenu=contextMenu->addMenu(DEBUG_MENU);

	toggleDebuggerAction=debugMenu->addAction(SHOW_DEBUGGER_ACTION_TEXT, this, SLOT(toggleDebugger()));
	debugContextMenu->addAction(toggleDebuggerAction);
}

void QtEmulator::buildSkinsMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* skinsMenu=new QMenu(SKINS_MENU);
	menuBar->addMenu(skinsMenu);
	QMenu* skinsContextMenu=contextMenu->addMenu(SKINS_MENU);

	delete skinsActionGroup;
	skinsActionGroup=new QActionGroup(this);
	skinsActionGroup->setExclusive(true);

	skinActions.clear();

	for(SkinMap::const_iterator skinIterator=skins.constBegin(); skinIterator!=skins.constEnd(); ++skinIterator)
	{
		QAction* skinAction=skinsMenu->addAction(skinIterator.key());
		skinsContextMenu->addAction(skinAction);
		skinsActionGroup->addAction(skinAction);
		skinAction->setCheckable(true);
		if(skinAction->text()==currentSkinName)
		{
			skinAction->setChecked(true);
		}
		skinActions.append(skinAction);
	}
	connect(skinsMenu, SIGNAL(triggered(QAction*)), this, SLOT(selectSkin(QAction*)));
}

void QtEmulator::buildHelpMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* helpMenu=new QMenu(HELP_MENU);
	menuBar->addMenu(helpMenu);
	QMenu* helpContextMenu=contextMenu->addMenu(HELP_MENU);

	QAction* aboutAction=helpMenu->addAction(ABOUT_ACTION_TEXT, this, SLOT(showAbout()));
	aboutAction->setMenuRole(QAction::AboutRole);
	helpContextMenu->addAction(aboutAction);

	QAction* showWebsiteAction=helpMenu->addAction(SHOW_WEBSITE_ACTION_TEXT, this, SLOT(showWebSite()));
	helpContextMenu->addAction(showWebsiteAction);
	QAction* showDocumentationAction=helpMenu->addAction(SHOW_DOCUMENTATION_ACTION_TEXT, this, SLOT(showDocumentation()));
	helpContextMenu->addAction(showDocumentationAction);
}

void QtEmulator::buildContextualQuit()
{
	contextMenu->addSeparator();
	contextMenu->addAction(QUIT_ACTION_TEXT, qApp, SLOT(quit()), QKeySequence::Quit);
}

void QtEmulator::buildComponents(const QtSkin& aSkin)
{
	centralWidget=new QWidget;
	screen=new QtScreen(aSkin, useFonts);
	keyboard=new QtKeyboard(aSkin, useHShiftClick, alwaysUseHShiftClick, hShiftDelay, showToolTips);
	backgroundImage=new QtBackgroundImage(aSkin, *screen, *keyboard, showCatalogMenus, closeCatalogMenus);
	connect(this, SIGNAL(catalogStateChanged()), backgroundImage, SLOT(onCatalogStateChanged()), Qt::BlockingQueuedConnection);
	QBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(centralWidget);
	setCentralWidget(centralWidget);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	centralLayout=new QHBoxLayout;
	centralLayout->setContentsMargins(0, 0, 0, 0);
	centralWidget->setLayout(centralLayout);
	centralLayout->addWidget(backgroundImage);
}

void QtEmulator::buildSerialPort()
{
	serialPort=new QtSerialPort;
}

void QtEmulator::buildDebugger()
{
	debugger=new QtDebugger(centralWidget, displayAsStack);
	debugger->setVisible(false);
	centralLayout->addWidget(debugger);
}

void QtEmulator::showContextMenu(const QPoint& aPoint)
{
	contextMenu->exec(mapToGlobal(aPoint));
}

void QtEmulator::startThreads()
{
	calculatorThread=new QtCalculatorThread(*this);
	calculatorThread->start();

	heartBeatThread=new QtHeartBeatThread();
	heartBeatThread->start();
}

void QtEmulator::stopThreads()
{
	if(heartBeatThread!=NULL && heartBeatThread->isRunning())
	{
		heartBeatThread->end();
		heartBeatThread->wait(THREAD_WAITING_TIME);
	}
	if(calculatorThread!=NULL && calculatorThread->isRunning())
	{
		calculatorThread->end();
		calculatorThread->wait(THREAD_WAITING_TIME);
	}
}

void QtEmulator::setPaths()
{
	QString applicationDir=QApplication::applicationDirPath()+'/';
#ifdef RESOURCES_DIR
	QString resourcesDir(applicationDir+RESOURCES_DIR);
#endif


	QStringList skinSearchPath;
	QStringList imageSearchPath;
	QStringList memorySearchPath;
	QStringList documentationSearchPath;
	QStringList fontsSearchPath;

	if(customDirectoryActive)
	{
		skinSearchPath << customDirectory.path();
		imageSearchPath << customDirectory.path();
		memorySearchPath << customDirectory.path();
		documentationSearchPath << customDirectory.path();
		fontsSearchPath << customDirectory.path();
	}
	else
	{
		memorySearchPath << userSettingsDirectoryName;
#ifdef Q_WS_MAC
		memorySearchPath << resourcesDir+MEMORY_DIRECTORY;
#endif
		memorySearchPath << applicationDir+MEMORY_DIRECTORY;
	}

	skinSearchPath << userSettingsDirectoryName;
	imageSearchPath << userSettingsDirectoryName;
	fontsSearchPath << userSettingsDirectoryName;

	if(development)
	{
		QString currentDir=QDir::currentPath()+'/';

		skinSearchPath << currentDir+SKIN_DIRECTORY;
		imageSearchPath << currentDir+IMAGE_DIRECTORY;
		documentationSearchPath << currentDir+DOCUMENTATION_DIRECTORY;
		fontsSearchPath << currentDir+FONTS_DIRECTORY;
		if(!customDirectoryActive)
		{
			memorySearchPath << currentDir+MEMORY_DIRECTORY;
		}
	}

#ifdef RESOURCES_DIR
	skinSearchPath << resourcesDir+SKIN_DIRECTORY;
	imageSearchPath << resourcesDir+IMAGE_DIRECTORY;
	documentationSearchPath << resourcesDir+DOCUMENTATION_DIRECTORY;
	fontsSearchPath << resourcesDir+FONTS_DIRECTORY;
#endif


	skinSearchPath << applicationDir+SKIN_DIRECTORY;
	imageSearchPath << applicationDir+IMAGE_DIRECTORY;
	documentationSearchPath << applicationDir+DOCUMENTATION_DIRECTORY;
	fontsSearchPath << applicationDir+FONTS_DIRECTORY;

	QDir::setSearchPaths(SKIN_FILE_TYPE, skinSearchPath);
	QDir::setSearchPaths(IMAGE_FILE_TYPE, imageSearchPath);
	QDir::setSearchPaths(MEMORY_FILE_TYPE, memorySearchPath);
	QDir::setSearchPaths(DOCUMENTATION_FILE_TYPE, documentationSearchPath);
	QDir::setSearchPaths(FONT_FILE_TYPE, fontsSearchPath);
}

QtSkin* QtEmulator::buildSkin(const QString& aSkinFilename) throw (QtSkinException)
{
	QFile skinFile(QString(SKIN_FILE_TYPE)+':'+aSkinFilename);
	if(skinFile.exists() && skinFile.open(QIODevice::ReadOnly))
	{
		return new QtSkin(skinFile);
	}
	else
	{
		return NULL;
	}
}

void QtEmulator::loadSettings()
{
	loadUserInterfaceSettings();
	loadKeyboardSettings();
	loadDisplaySettings();
	loadCustomDirectorySettings();
	loadLastMemoryFileSettings();
	loadSerialPortSettings();
	loadToolsSettings();
	checkCustomDirectory();
}

void QtEmulator::loadUserInterfaceSettings()
{
	settings.beginGroup(WINDOWS_SETTINGS_GROUP);
	move(settings.value(WINDOWS_POSITION_SETTING, QPoint(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)).toPoint());
	titleBarVisible=settings.value(WINDOWS_TITLEBAR_VISIBLE_SETTING, true).toBool();
	debuggerVisible=settings.value(DEBUGGER_VISIBLE_SETTING, true).toBool();
	settings.endGroup();

	settings.beginGroup(SKIN_SETTINGS_GROUP);
	currentSkinName=settings.value(LAST_SKIN_SETTING, "").toString();
	settings.endGroup();
}

void QtEmulator::loadKeyboardSettings()
{
	settings.beginGroup(KEYBOARD_SETTINGS_GROUP);
	useHShiftClick=settings.value(USE_HSHIFT_CLICK_SETTING, DEFAULT_USE_HSHIFT_CLICK).toBool();
	alwaysUseHShiftClick=settings.value(ALWAYS_USE_HSHIFT_CLICK_SETTING, DEFAULT_ALWAYS_USE_HSHIFT_CLICK).toBool();
	hShiftDelay=settings.value(HSHIFT_DELAY_SETTING, DEFAULT_HSHIFT_DELAY).toInt();
	showToolTips=settings.value(SHOW_TOOLTIPS_SETTING, DEFAULT_SHOW_TOOLTIPS_SETTING).toBool();
	settings.endGroup();
}

void QtEmulator::loadDisplaySettings()
{
	settings.beginGroup(DISPLAY_SETTINGS_GROUP);
	useFonts=settings.value(USE_FONTS_SETTING, DEFAULT_USE_FONTS_SETTING).toBool();
	showCatalogMenus=settings.value(SHOW_CATALOG_MENUS_SETTING, DEFAULT_SHOW_CATALOG_MENUS_SETTING).toBool();
	closeCatalogMenus=settings.value(CLOSE_CATALOG_MENUS_SETTING, DEFAULT_CLOSE_CATALOG_MENUS_SETTING).toBool();
	displayAsStack=settings.value(DISPLAY_AS_STACK_SETTING, DEFAULT_DISPLAY_AS_STACK).toBool();
	settings.endGroup();
}


void QtEmulator::loadCustomDirectorySettings()
{
	settings.beginGroup(CUSTOM_DIRECTORY_SETTINGS_GROUP);
	customDirectoryActive=settings.value(CUSTOM_DIRECTORY_ACTIVE_SETTING, false).toBool();
	customDirectory.setPath(settings.value(CUSTOM_DIRECTORY_NAME_SETTING, "").toString());
	settings.endGroup();
}

void QtEmulator::loadLastMemoryFileSettings()
{
	settings.beginGroup(LAST_MEMORYFILE_SETTINGS_GROUP);
	lastMemoryFileActive=settings.value(LAST_MEMORYFILE_ACTIVE_SETTING, false).toBool();
	lastMemoryFile=settings.value(LAST_MEMORYFILE_SETTING, "").toString();
	settings.endGroup();
}

void QtEmulator::loadSerialPortSettings()
{
	settings.beginGroup(SERIAL_PORT_SETTINGS_GROUP);
	QString serialPortName=settings.value(SERIAL_PORT_NAME_SETTING, "").toString();
	serialPort->setSerialPortName(serialPortName);
	settings.endGroup();
}

void QtEmulator::loadToolsSettings()
{
	settings.beginGroup(TOOLS_SETTINGS_GROUP);
	toolsActive=settings.value(TOOLS_ACTIVE_SETTING, false).toBool();
	tools.setPath(settings.value(TOOLS_NAME_SETTING, "").toString());
	settings.endGroup();
	forwardToolsSettings();
}

void QtEmulator::forwardToolsSettings()
{
	QDir dir;

	if(toolsActive)
	{
		dir=tools;
	}
	else
	{
		dir = QDir(QCoreApplication::applicationDirPath());
#ifdef Q_WS_MAC
		dir.cdUp();
		dir.cd("Resources");
#endif
		dir.cd("tools");
	}

#ifdef Q_WS_WIN
	QString assembler("wp34s_asm.exe");
#else
	QString assembler("wp34s_asm.pl");
#endif
	forward_set_assembler(dir.absoluteFilePath(assembler).toStdString().c_str());
}

void QtEmulator::saveSettings()
{
    saveUserInterfaceSettings();
    saveKeyboardSettings();
    saveDisplaySettings();
    saveCustomDirectorySettings();
    saveLastMemoryFileSettings();
    saveSerialPortSettings();

	settings.sync();
}

void QtEmulator::saveUserInterfaceSettings()
{
    settings.beginGroup(WINDOWS_SETTINGS_GROUP);
    settings.setValue(WINDOWS_POSITION_SETTING, pos());
    settings.setValue(WINDOWS_TITLEBAR_VISIBLE_SETTING, titleBarVisible);
    settings.setValue(DEBUGGER_VISIBLE_SETTING, debuggerVisible);
    settings.endGroup();

    settings.beginGroup(SKIN_SETTINGS_GROUP);
    settings.setValue(LAST_SKIN_SETTING, currentSkinName);
    settings.endGroup();
}

void QtEmulator::saveKeyboardSettings()
{
    settings.beginGroup(KEYBOARD_SETTINGS_GROUP);
    settings.setValue(USE_HSHIFT_CLICK_SETTING, keyboard->isUseHShiftClick());
    settings.setValue(ALWAYS_USE_HSHIFT_CLICK_SETTING, keyboard->isAlwaysUseHShiftClick());
    settings.setValue(HSHIFT_DELAY_SETTING, keyboard->getHShiftDelay());
    settings.setValue(SHOW_TOOLTIPS_SETTING, keyboard->isShowToolTips());
    settings.endGroup();
}

void QtEmulator::saveDisplaySettings()
{
    settings.beginGroup(DISPLAY_SETTINGS_GROUP);
    settings.setValue(USE_FONTS_SETTING, screen->isUseFonts());
    settings.setValue(SHOW_CATALOG_MENUS_SETTING, backgroundImage->isShowCatalogMenu());
    settings.setValue(CLOSE_CATALOG_MENUS_SETTING, backgroundImage->isCloseCatalogMenu());
    settings.setValue(DISPLAY_AS_STACK_SETTING, debugger->isDisplayAsStack());
    settings.endGroup();
}


void QtEmulator::saveCustomDirectorySettings()
{
	settings.beginGroup(CUSTOM_DIRECTORY_SETTINGS_GROUP);
	settings.setValue(CUSTOM_DIRECTORY_ACTIVE_SETTING, customDirectoryActive);
	settings.setValue(CUSTOM_DIRECTORY_NAME_SETTING, customDirectory.path());
	settings.endGroup();
}

void QtEmulator::saveLastMemoryFileSettings()
{
	settings.beginGroup(LAST_MEMORYFILE_SETTINGS_GROUP);
	settings.setValue(LAST_MEMORYFILE_ACTIVE_SETTING, lastMemoryFileActive);
	settings.setValue(LAST_MEMORYFILE_SETTING, lastMemoryFile);
	settings.endGroup();
}

void QtEmulator::saveSerialPortSettings()
{
    settings.beginGroup(SERIAL_PORT_SETTINGS_GROUP);
    QString serialPortName=serialPort->getSerialPortName();
    settings.setValue(SERIAL_PORT_NAME_SETTING, serialPortName);
    serialPort->setSerialPortName(serialPortName);
    settings.endGroup();
}

void QtEmulator::saveToolsSettings()
{
	settings.beginGroup(TOOLS_SETTINGS_GROUP);
	settings.setValue(TOOLS_ACTIVE_SETTING, toolsActive);
	settings.setValue(TOOLS_NAME_SETTING, tools.path());
	settings.endGroup();
}

void QtEmulator::loadMemory()
{
	loadState();
	loadBackup();
	loadLibrary();
	after_state_load();
}

void QtEmulator::loadState()
{
	QFile memoryFile;
	if(lastMemoryFileActive)
	{
		memoryFile.setFileName(lastMemoryFile);
	}
	else
	{
		memoryFile.setFileName(QString(MEMORY_FILE_TYPE)+':'+STATE_FILENAME);
	}
	currentMemoryFile=QFileInfo(memoryFile).canonicalFilePath();
	if(currentMemoryFile.isEmpty())
	{
		currentMemoryFile=getMemoryPath(STATE_FILENAME);
	}
	if(!memoryFile.exists() || !memoryFile.open(QIODevice::ReadOnly))
	{
		memoryWarning("Cannot find or cannot open "+memoryFile.fileName());
		return;
	}

	int memorySize=get_memory_size();
	if(memoryFile.size()!=memorySize)
	{
		memoryWarning(memoryFile.fileName()+" expected size is "+QString::number(memorySize)
		+" but file size is "+QString::number(memoryFile.size()));
		return;
	}

	QDataStream dataStream(&memoryFile);
	int reallyRead=dataStream.readRawData(get_memory(), memorySize);
	if(reallyRead!=memorySize)
	{
		memoryWarning("Error whilst reading "+memoryFile.fileName());
		return;
	}
}

void QtEmulator::loadBackup()
{
	QFile backupFile(QString(MEMORY_FILE_TYPE)+':'+BACKUP_FILENAME);
	if(!backupFile.exists() || !backupFile.open(QIODevice::ReadOnly))
	{
		return;
	}

	int backupSize=get_backup_size();
	QDataStream dataStream(&backupFile);
	dataStream.readRawData(get_backup(), backupSize);
}

void QtEmulator::loadLibrary()
{
	QFile libraryFile(QString(MEMORY_FILE_TYPE)+':'+LIBRARY_FILENAME);
	if(!libraryFile.exists() || !libraryFile.open(QIODevice::ReadOnly))
	{
		return;
	}

	int librarySize=get_user_flash_size();
	QDataStream dataStream(&libraryFile);
	dataStream.readRawData(get_user_flash(), librarySize);
}

void QtEmulator::saveMemory()
{
	saveState();
}

void QtEmulator::saveState()
{
	prepare_memory_save();
	QFile memoryFile(getMemoryPath(STATE_FILENAME));
	if(!memoryFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		memoryWarning("Cannot open "+memoryFile.fileName());
		return;
	}

	QDataStream dataStream(&memoryFile);
	int memorySize=get_memory_size();
	int reallyWritten=dataStream.writeRawData(get_memory(), memorySize);
	if(reallyWritten!=memorySize)
	{
		memoryWarning("Cannot write "+memoryFile.fileName());
	}
}

void QtEmulator::saveBackup()
{
}

void QtEmulator::saveLibrary()
{
}

QString QtEmulator::getMemoryPath(const QString& aMemoryFilename) const
{
	if(lastMemoryFileActive)
	{
		return lastMemoryFile;
	}
	else if(customDirectoryActive)
	{
		return customDirectory.path()+'/'+aMemoryFilename;
	}
	else
	{
		return userSettingsDirectoryName+'/'+aMemoryFilename;
	}
}

char* QtEmulator::getRegionPath(int aRegionIndex)
{
	QString regionPath(getMemoryPath(getRegionFileName(aRegionIndex)));
	currentRegionPath=regionPath.toAscii();
	return currentRegionPath.data();
}

extern "C"
{

char* get_region_path_adapter(int aRegionIndex)
{
	return currentEmulator->getRegionPath(aRegionIndex);
}

}

QString QtEmulator::getRegionFileName(int aRegionIndex) const
{
	return aRegionIndex == get_region_backup_index() ? BACKUP_FILENAME : LIBRARY_FILENAME;
}

void QtEmulator::resetUserMemory()
{
	bool removed=true;
	QFile memoryFile(getMemoryPath(STATE_FILENAME));
	if(memoryFile.exists())
	{
		removed &= memoryFile.remove();
	}

	QFile backupFile(getMemoryPath(BACKUP_FILENAME));
	if(backupFile.exists())
	{
		removed &= backupFile.remove();
	}

	QFile libraryFile(getMemoryPath(LIBRARY_FILENAME));
	if(libraryFile.exists())
	{
		removed &= libraryFile.remove();
	}

	reset_wp34s();

	if(!removed)
	{
		memoryWarning("Cannot reset user memory", false);
	}

	updateScreen();
}

void QtEmulator::skinError(const QString& aMessage, bool aFatalFlag)
{
	QMessageBox messageBox;
	messageBox.setIcon(aFatalFlag?QMessageBox::Critical:QMessageBox::Warning);
	messageBox.setText(aFatalFlag?"Cannot revert to previous skin":"Cannot switch to selected skin");
	messageBox.setInformativeText(aMessage);
	messageBox.setStandardButtons(QMessageBox::Ok);
	messageBox.exec();
	if(aFatalFlag)
	{
		qApp->exit(-1);
	}
}

void QtEmulator::memoryWarning(const QString& aMessage, bool aResetFlag)
{
	QMessageBox messageBox;
	messageBox.setIcon(QMessageBox::Warning);
	messageBox.setText("Error with memory file");
	QString message=aMessage;
	if(lastMemoryFileActive && !aResetFlag)
	{
		message += "\nCannot use " + lastMemoryFile;
		lastMemoryFileActive=false;
	}
	messageBox.setInformativeText(message);
	if(aResetFlag)
	{
		messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Reset);
	}
	else
	{
		messageBox.setStandardButtons(QMessageBox::Ok);
	}
	messageBox.setDefaultButton(QMessageBox::Ok);
	if(messageBox.exec()==QMessageBox::Reset)
	{
		resetUserMemory();
	}
}

void QtEmulator::setInitialSkin() throw (QtSkinException)
{
	findSkins();
	QString skinName=currentSkinName;
	QString skinFilename=skins.value(skinName);

	// We clear now so if there is an error, it will be empty next time
	currentSkinName.clear();

	QtSkin* skin=NULL;
	try
	{
		skin=buildSkin(skinFilename);
	}
	catch(QtSkinException& exception)
	{
		skinError("Cannot read skin "+skinName+": "+QString(exception.what()), false);
	}
	if(skin==NULL)
	{
		for(SkinMap::const_iterator skinIterator=skins.constBegin(); skinIterator!=skins.constEnd(); ++skinIterator)
		{
			if(skinName!=skinIterator.key())
			{
				try
				{
					skin=buildSkin(skinIterator.value());
				}
				catch(QtSkinException& exception)
				{
					skinError("Cannot read skin "+skinIterator.key()+": "+QString(exception.what()), false);
				}
				if(skin!=NULL)
				{
					break;
				}
			}
		}

		if(skin==NULL)
		{
			throw *(new QtSkinException("Cannot find a skin"));
		}
	}
	buildComponents(*skin);
	delete skin;

	// If everything went ok, we set the saved skin name to the current one
	currentSkinName=skinName;
}

void QtEmulator::setSkin(const QString& aSkinName) throw (QtSkinException)
{
	QString skinFilename=skins.value(aSkinName);
	if(skinFilename.isEmpty())
	{
		throw *(new QtSkinException("Unknow skin "+aSkinName));
	}

	QtSkin* skin=buildSkin(skinFilename);
	keyboard->setSkin(*skin);
	screen->setSkin(*skin);
	backgroundImage->setSkin(*skin);
	delete skin;

	currentSkinName=aSkinName;
	layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void QtEmulator::findSkins()
{
	skins.clear();
	QStringList skinPath=QDir::searchPaths(SKIN_FILE_TYPE);
	QStringList nameFilters(QString("*.")+SKIN_SUFFIX);
	for (QStringList::const_iterator skinPathIterator = skinPath.constBegin(); skinPathIterator != skinPath.constEnd(); ++skinPathIterator)
	{
		QDir currentDir(*skinPathIterator);
		QFileInfoList fileInfos=currentDir.entryInfoList(nameFilters);
		for(QFileInfoList::const_iterator fileInfoIterator = fileInfos.constBegin(); fileInfoIterator != fileInfos.constEnd(); ++fileInfoIterator)
		{
			skins[fileInfoIterator->baseName()]=fileInfoIterator->fileName();
		}
	}
}

void QtEmulator::toggleDebugger()
{
	setDebuggerVisible(!debuggerVisible);
}

void QtEmulator::setDebuggerVisible(bool aDebuggerVisible)
{
	debuggerVisible=aDebuggerVisible;
	if(debuggerVisible)
	{
		debugger->setVisible(true);
		toggleDebuggerAction->setText(HIDE_DEBUGGER_ACTION_TEXT);
	}
	else
	{
		debugger->setVisible(false);
		toggleDebuggerAction->setText(SHOW_DEBUGGER_ACTION_TEXT);
	}
	centralLayout->invalidate();
	centralLayout->update();
	setFixedSize(sizeHint());
}

void QtEmulator::onCatalogStateChanged()
{
	emit catalogStateChanged();
}

void QtEmulator::showCatalogMenu()
{
	backgroundImage->showCatalogMenu(true);
}

void QtEmulator::showMessage(const char* title, const char* message)
{
	QMessageBox messageBox;
	messageBox.setWindowTitle(title);
	messageBox.setText(message);
	messageBox.exec();
}

void QtEmulator::reload()
{
	loadMemory();
	init_calculator();
}

void QtEmulator::open()
{
	QString filename=QFileDialog::getOpenFileName(this, "Load State File As", QString(), STATE_FILE_FILTER);
	if(!filename.isEmpty())
	{
		setLastMemoryFile(filename);
		loadMemory();
		init_calculator();
	}
}

void QtEmulator::save()
{
	saveMemory();
}

void QtEmulator::saveAs()
{
	QString filename=QFileDialog::getSaveFileName(this, "Save State File As", QString(), STATE_FILE_FILTER);
	if(!filename.isEmpty())
	{
		setLastMemoryFile(filename);
		saveMemory();
	}
}

void QtEmulator::importState()
{
	QString filename=QFileDialog::getOpenFileName(this, "Import", QString(), IMPORT_FILE_FILTER);
	if(!filename.isEmpty())
	{
		forward_import(filename.toStdString().c_str());
	}
}

void QtEmulator::exportState()
{
	QString filename=QFileDialog::getSaveFileName(this, "Export", QString(), EXPORT_FILE_FILTER);
	if(!filename.isEmpty())
	{
		forward_export(filename.toStdString().c_str());
	}
}

void QtEmulator::useDefaultState()
{
	lastMemoryFileActive=false;
	loadMemory();
	init_calculator();
}

void QtEmulator::setLastMemoryFile(QString& aFilename)
{
	lastMemoryFileActive=true;
	lastMemoryFile=aFilename;
}


extern "C"
{

void shutdown_adapter()
{
	QCoreApplication::quit();
}

}

