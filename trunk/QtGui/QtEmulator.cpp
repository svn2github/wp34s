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

QtEmulator* currentEmulator;

QtEmulator::QtEmulator(QApplication& anApplication)
: application(anApplication), skinsActionGroup(NULL)
{
	debug=application.arguments().contains(DEBUG_OPTION);

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
}

QtEmulator::~QtEmulator()
{
	delete skinsActionGroup;
}

void QtEmulator::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event)

	saveSettings();
	saveMemory();
}

QtKeyboard& QtEmulator::getKeyboard() const
{
	return *keyboard;
}

QtScreen& QtEmulator::getScreen() const
{
	return *screen;
}

void QtEmulator::updateScreen()
{
	emit screenChanged();
}

void QtEmulator::editPreferences()
{
	QtPreferencesDialog preferencesDialog(customDirectoryActive, customDirectory.path(), this);
	int result=preferencesDialog.exec();
	if(result==QDialog::Accepted)
	{
		customDirectoryActive=preferencesDialog.isCustomDirectoryActive();
		customDirectory.setPath(preferencesDialog.getCustomDirectoryName());
		checkCustomDirectory();
		saveCustomDirectorySettings();
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
	mainMenu->addAction(QUIT_ACTION_TEXT, &application, SLOT(quit()));
#endif
}

void QtEmulator::buildEditMenu()
{
	QMenuBar* menuBar=this->menuBar();
	QMenu* editMenu=new QMenu(EDIT_MENU);
	menuBar->addMenu(editMenu);

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
}

void QtEmulator::buildComponents(const QtSkin& aSkin)
{
	screen=new QtScreen(aSkin);
	keyboard=new QtKeyboard(aSkin);
	backgroundImage=new QtBackgroundImage(aSkin, *screen, *keyboard);
}

void QtEmulator::startThreads()
{
	calculatorThread=new QtCalculatorThread(*keyboard);
	calculatorThread->start();

	heartBeatThread=new QtHeartBeatThread();
	heartBeatThread->start();
}

void QtEmulator::setPaths()
{
	QString applicationDir=QApplication::applicationDirPath();
#ifdef Q_WS_MAC
	QString resourcesDir(applicationDir+RESOURCES_DIR);
#endif

	QStringList skinSearchPath;
	QStringList imageSearchPath;
	QStringList memorySearchPath;

	if(customDirectoryActive)
	{
		skinSearchPath << customDirectory.path();
		imageSearchPath << customDirectory.path();
		memorySearchPath << customDirectory.path();
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

	if(debug)
	{
		QString currentDir=QDir::currentPath()+'/';

		skinSearchPath << currentDir+SKIN_DIRECTORY;
		imageSearchPath << currentDir+IMAGE_DIRECTORY;
		if(!customDirectoryActive)
		{
			memorySearchPath << currentDir+MEMORY_DIRECTORY;
		}
	}

#ifdef Q_WS_MAC
	skinSearchPath << resourcesDir+SKIN_DIRECTORY;
	imageSearchPath << resourcesDir+IMAGE_DIRECTORY;
#endif

	skinSearchPath << applicationDir+SKIN_DIRECTORY;
	imageSearchPath << applicationDir+IMAGE_DIRECTORY;

	QDir::setSearchPaths(SKIN_FILE_TYPE, skinSearchPath);
	QDir::setSearchPaths(IMAGE_FILE_TYPE, imageSearchPath);
	QDir::setSearchPaths(MEMORY_FILE_TYPE, memorySearchPath);
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
	 settings.beginGroup(WINDOWS_SETTINGS_GROUP);
	 move(settings.value(WINDOWS_POSITION_SETTING, QPoint(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)).toPoint());
	 settings.endGroup();

	 settings.beginGroup(SKIN_SETTINGS_GROUP);
	 currentSkinName=settings.value(LAST_SKIN_SETTING, "").toString();
	 settings.endGroup();

	 settings.beginGroup(CUSTOM_DIRECTORY_SETTINGS_GROUP);
	 customDirectoryActive=settings.value(CUSTOM_DIRECTORY_ACTIVE_SETTING, false).toBool();
	 customDirectory.setPath(settings.value(CUSTOM_DIRECTORY_NAME_SETTING, "").toString());
	 settings.endGroup();

	 checkCustomDirectory();
}

void QtEmulator::saveSettings()
{
    settings.beginGroup(WINDOWS_SETTINGS_GROUP);
    settings.setValue(WINDOWS_POSITION_SETTING, pos());
    settings.endGroup();

    settings.beginGroup(SKIN_SETTINGS_GROUP);
    settings.setValue(LAST_SKIN_SETTING, currentSkinName);
    settings.endGroup();

    saveCustomDirectorySettings();

	settings.sync();
}

void QtEmulator::saveCustomDirectorySettings()
{
	settings.beginGroup(CUSTOM_DIRECTORY_SETTINGS_GROUP);
	settings.setValue(CUSTOM_DIRECTORY_ACTIVE_SETTING, customDirectoryActive);
	settings.setValue(CUSTOM_DIRECTORY_NAME_SETTING, customDirectory.path());
	settings.endGroup();
}

void QtEmulator::loadMemory()
{
	QFile memoryFile(QString(MEMORY_FILE_TYPE)+':'+NON_VOLATILE_MEMORY_FILENAME);
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

	for (int i = 0; i < get_number_of_flash_regions(); ++i )
	{
		loadMemoryRegion(i);
	}
}

bool QtEmulator::loadMemoryRegion(int aRegionIndex)
{
	QFile memoryRegionFile(QString(MEMORY_FILE_TYPE)+':'+getRegionFileName(aRegionIndex));
	if(!memoryRegionFile.exists())
	{
		if(aRegionIndex==0)
		{
			// If we cannot read the saved backup flash region,
			// we behave as if the main memory had already been saved in it
			fast_backup_to_flash();
		}
		return true;
	}

	if(!memoryRegionFile.open(QIODevice::ReadOnly))
	{
		memoryWarning("Cannot open "+memoryRegionFile.fileName());
		return false;
	}

	int memoryRegionSize=get_flash_region_size();
	if(memoryRegionFile.size()!=memoryRegionSize)
	{
		memoryWarning(memoryRegionFile.fileName()+" expected size is "+QString::number(memoryRegionSize)
		+" but file size is "+QString::number(memoryRegionFile.size()));
		return false;
	}

	QDataStream dataStream(&memoryRegionFile);
	char* region=get_flash_region(aRegionIndex);
	memset( region, FLASH_REGION_DEFAULT_VALUE, memoryRegionSize );
	int reallyRead=dataStream.readRawData(region, memoryRegionSize);
	if(reallyRead!=memoryRegionSize)
	{
		memoryWarning("Error whilst reading "+memoryRegionFile.fileName());
		return false;
	}

	return true;
}

void QtEmulator::saveMemory()
{
	prepare_memory_save();
	QFile memoryFile(getMemoryPath(NON_VOLATILE_MEMORY_FILENAME));
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

QString QtEmulator::getRegionFileName(int aRegionIndex) const
{
	QString regionFileName(REGION_FILENAME_PATTERN);
	if(aRegionIndex==0)
	{
		return regionFileName.arg('R');
	}
	else
	{
		return regionFileName.arg(QString::number(aRegionIndex));
	}
}

void QtEmulator::resetUserMemory()
{
	bool removed=true;
	QFile memoryFile(getMemoryPath(NON_VOLATILE_MEMORY_FILENAME));
	if(memoryFile.exists())
	{
		removed &= memoryFile.remove();
	}

	for (int i = 0; i < get_number_of_flash_regions(); ++i )
	{
		QFile regionMemoryFile(getRegionPath(i));
		if(regionMemoryFile.exists())
		{
			removed &= regionMemoryFile.remove();
		}
	}

	if(!removed)
	{
		memoryWarning("Cannot reset user memory", false);
	}
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
