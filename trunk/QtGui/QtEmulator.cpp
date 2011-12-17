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

QtEmulator::QtEmulator(QApplication& anApplication)
: application(anApplication), calculatorThread(NULL), heartBeatThread(NULL), skinsActionGroup(NULL)
{
	debug=application.arguments().contains(DEBUG_OPTION);
	development=application.arguments().contains(DEVELOPMENT_OPTION);

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
	buildMenuBar();

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(backgroundImage);
	setCentralWidget(backgroundImage);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	connect(this, SIGNAL(screenChanged()), backgroundImage, SLOT(updateScreen()));

	setWindowTitle(QApplication::translate("wp34s", "WP34s"));
	currentEmulator = this;

	loadMemory();
	startThreads();
	active=true;
}

QtEmulator::~QtEmulator()
{
	if(active)
	{
		quit();
	}
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

void QtEmulator::updateScreen()
{
	emit screenChanged();
}

void QtEmulator::editPreferences()
{
	QtPreferencesDialog preferencesDialog(customDirectoryActive,
			customDirectory.path(),
			keyboard->getHShiftDelay(),
			serialPort->getSerialPortName(), this);
	int result=preferencesDialog.exec();
	if(result==QDialog::Accepted)
	{
		customDirectoryActive=preferencesDialog.isCustomDirectoryActive();
		customDirectory.setPath(preferencesDialog.getCustomDirectoryName());
		checkCustomDirectory();
		saveCustomDirectorySettings();

		keyboard->setHShiftDelay(preferencesDialog.getHShiftDelay());
		saveKeyboardSettings();

		QString serialPortName=preferencesDialog.getSerialPortName();
		serialPort->setSerialPortName(serialPortName);
		saveSerialPortSettings();

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
	application.clipboard()->setText(get_formatted_displayed_number());
}

void QtEmulator::copyTextLine()
{
	application.clipboard()->setText(get_displayed_text());
}

void QtEmulator::copyImage()
{
	screen->copy(*backgroundImage, *application.clipboard());
}

void QtEmulator::pasteNumber()
{
	QtNumberPaster::paste(application.clipboard()->text(), *keyboard);
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
		}
		catch(QtSkinException& exception)
		{
			skinError(exception.what(), true);
		}
	}
}

void QtEmulator::buildMenuBar()
{
	buildMainMenu();
	buildEditMenu();
	buildSkinsMenu();
	buildHelpMenu();
}

void QtEmulator::buildMainMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* mainMenu=new QMenu(MAIN_MENU);
	menuBar->addMenu(mainMenu);

	mainMenu->addAction(RESET_ACTION_TEXT, this, SLOT(confirmReset()));

#ifndef Q_WS_MAC
	mainMenu->addSeparator();
	mainMenu->addAction(QUIT_ACTION_TEXT, &application, SLOT(quit()), QKeySequence::Quit);
#endif
}

void QtEmulator::buildEditMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* editMenu=new QMenu(EDIT_MENU);
	menuBar->addMenu(editMenu);

	editMenu->addAction(COPY_NUMBER_ACTION_TEXT, this, SLOT(copyNumber()), QKeySequence::Copy);
	editMenu->addAction(COPY_TEXTLINE_ACTION_TEXT, this, SLOT(copyTextLine()));
	editMenu->addAction(COPY_IMAGE_ACTION_TEXT, this, SLOT(copyImage()));
	editMenu->addAction(PASTE_NUMBER_ACTION_TEXT, this, SLOT(pasteNumber()), QKeySequence::Paste);

#ifndef Q_WS_MAC
	editMenu->addSeparator();
#endif
	QAction* preferencesAction=editMenu->addAction(PREFERENCES_ACTION_TEXT, this, SLOT(editPreferences()));
	preferencesAction->setMenuRole(QAction::PreferencesRole);
}

void QtEmulator::buildSkinsMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* skinsMenu=new QMenu(SKINS_MENU);
	menuBar->addMenu(skinsMenu);

	delete skinsActionGroup;
	skinsActionGroup=new QActionGroup(this);
	skinsActionGroup->setExclusive(true);

	for(SkinMap::const_iterator skinIterator=skins.constBegin(); skinIterator!=skins.constEnd(); ++skinIterator)
	{
		QAction* skinAction=skinsMenu->addAction(skinIterator.key());
		skinsActionGroup->addAction(skinAction);
		skinAction->setCheckable(true);
		if(skinAction->text()==currentSkinName)
		{
			skinAction->setChecked(true);
		}
	}
	connect(skinsMenu, SIGNAL(triggered(QAction*)), this, SLOT(selectSkin(QAction*)));
}

void QtEmulator::buildHelpMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* helpMenu=new QMenu(HELP_MENU);
	menuBar->addMenu(helpMenu);

	QAction* aboutAction=helpMenu->addAction(ABOUT_ACTION_TEXT, this, SLOT(showAbout()));
	aboutAction->setMenuRole(QAction::AboutRole);

	helpMenu->addAction(SHOW_WEBSITE_ACTION_TEXT, this, SLOT(showWebSite()));
	helpMenu->addAction(SHOW_DOCUMENTATION_ACTION_TEXT, this, SLOT(showDocumentation()));
}

void QtEmulator::buildComponents(const QtSkin& aSkin)
{
	screen=new QtScreen(aSkin);
	keyboard=new QtKeyboard(aSkin, hShiftDelay);
	backgroundImage=new QtBackgroundImage(aSkin, *screen, *keyboard);
}

void QtEmulator::buildSerialPort()
{
	serialPort=new QtSerialPort;
}

void QtEmulator::startThreads()
{
	calculatorThread=new QtCalculatorThread(*keyboard);
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
	QString applicationDir=QApplication::applicationDirPath();
#ifdef RESOURCES_DIR
	QString resourcesDir(applicationDir+RESOURCES_DIR);
#endif


	QStringList skinSearchPath;
	QStringList imageSearchPath;
	QStringList memorySearchPath;
	QStringList documentationSearchPath;

	if(customDirectoryActive)
	{
		skinSearchPath << customDirectory.path();
		imageSearchPath << customDirectory.path();
		memorySearchPath << customDirectory.path();
		documentationSearchPath << customDirectory.path();
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

	if(development)
	{
		QString currentDir=QDir::currentPath()+'/';

		skinSearchPath << currentDir+SKIN_DIRECTORY;
		imageSearchPath << currentDir+IMAGE_DIRECTORY;
		documentationSearchPath << currentDir+DOCUMENTATION_DIRECTORY;
		if(!customDirectoryActive)
		{
			memorySearchPath << currentDir+MEMORY_DIRECTORY;
		}
	}

#ifdef RESOURCES_DIR
	skinSearchPath << resourcesDir+SKIN_DIRECTORY;
	imageSearchPath << resourcesDir+IMAGE_DIRECTORY;
	documentationSearchPath << resourcesDir+DOCUMENTATION_DIRECTORY;
#endif


	skinSearchPath << applicationDir+SKIN_DIRECTORY;
	imageSearchPath << applicationDir+IMAGE_DIRECTORY;
	documentationSearchPath << applicationDir+DOCUMENTATION_DIRECTORY;

	QDir::setSearchPaths(SKIN_FILE_TYPE, skinSearchPath);
	QDir::setSearchPaths(IMAGE_FILE_TYPE, imageSearchPath);
	QDir::setSearchPaths(MEMORY_FILE_TYPE, memorySearchPath);
	QDir::setSearchPaths(DOCUMENTATION_FILE_TYPE, documentationSearchPath);
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
	loadCustomDirectorySettings();
	loadSerialPortSettings();

	checkCustomDirectory();
}

void QtEmulator::loadUserInterfaceSettings()
{
	settings.beginGroup(WINDOWS_SETTINGS_GROUP);
	move(settings.value(WINDOWS_POSITION_SETTING, QPoint(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)).toPoint());
	settings.endGroup();

	settings.beginGroup(SKIN_SETTINGS_GROUP);
	currentSkinName=settings.value(LAST_SKIN_SETTING, "").toString();
	settings.endGroup();
}

void QtEmulator::loadKeyboardSettings()
{
	settings.beginGroup(KEYBOARD_SETTINGS_GROUP);
	hShiftDelay=settings.value(HSHIFT_DELAY_SETTING, DEFAULT_HSHIFT_DELAY).toInt();
	settings.endGroup();
}

void QtEmulator::loadCustomDirectorySettings()
{
	settings.beginGroup(CUSTOM_DIRECTORY_SETTINGS_GROUP);
	customDirectoryActive=settings.value(CUSTOM_DIRECTORY_ACTIVE_SETTING, false).toBool();
	customDirectory.setPath(settings.value(CUSTOM_DIRECTORY_NAME_SETTING, "").toString());
	settings.endGroup();
}

void QtEmulator::loadSerialPortSettings()
{
	settings.beginGroup(SERIAL_PORT_SETTINGS_GROUP);
	QString serialPortName=settings.value(SERIAL_PORT_NAME_SETTING, "").toString();
	serialPort->setSerialPortName(serialPortName);
	settings.endGroup();
}

void QtEmulator::saveSettings()
{
    saveUserInterfaceSettings();
    saveKeyboardSettings();
    saveCustomDirectorySettings();
    saveSerialPortSettings();

	settings.sync();
}

void QtEmulator::saveUserInterfaceSettings()
{
    settings.beginGroup(WINDOWS_SETTINGS_GROUP);
    settings.setValue(WINDOWS_POSITION_SETTING, pos());
    settings.endGroup();

    settings.beginGroup(SKIN_SETTINGS_GROUP);
    settings.setValue(LAST_SKIN_SETTING, currentSkinName);
    settings.endGroup();
}

void QtEmulator::saveKeyboardSettings()
{
    settings.beginGroup(KEYBOARD_SETTINGS_GROUP);
    settings.setValue(HSHIFT_DELAY_SETTING, keyboard->getHShiftDelay());
    settings.endGroup();
}

void QtEmulator::saveCustomDirectorySettings()
{
	settings.beginGroup(CUSTOM_DIRECTORY_SETTINGS_GROUP);
	settings.setValue(CUSTOM_DIRECTORY_ACTIVE_SETTING, customDirectoryActive);
	settings.setValue(CUSTOM_DIRECTORY_NAME_SETTING, customDirectory.path());
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

void QtEmulator::loadMemory()
{
	loadState();
	loadBackup();
	loadLibrary();
}

void QtEmulator::loadState()
{
	QFile memoryFile(QString(MEMORY_FILE_TYPE)+':'+STATE_FILENAME);
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

	after_state_load();
}

void QtEmulator::loadBackup()
{

}

void QtEmulator::loadLibrary()
{

}

void QtEmulator::saveMemory()
{
	saveState();
	saveBackup();
	saveLibrary();
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
		return;
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
	if(customDirectoryActive)
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
		application.exit(-1);
	}
}

void QtEmulator::memoryWarning(const QString& aMessage, bool aResetFlag)
{
	QMessageBox messageBox;
	messageBox.setIcon(QMessageBox::Warning);
	messageBox.setText("Error with memory files");
	messageBox.setInformativeText(aMessage);
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

