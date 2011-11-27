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

#include "QtEmulator.h"
#include "QtEmulatorAdapter.h"

QtEmulator* currentEmulator;

QtEmulator::QtEmulator()
{
#ifdef Q_WS_MAC
	QSettings::Format format=QSettings::NativeFormat;
#else
	QSettings::Format format=QSettings::IniFormat;
#endif
	QSettings userSettings(format, QSettings::UserScope, QString(ORGANIZATION_NAME), QString(APPLICATION_NAME));
	userSettingsDirectoryName=QFileInfo(userSettings.fileName()).dir().path();

#ifdef Q_WS_MAC
	userSettingsDirectoryName.append('/').append(APPLICATION_NAME);
#endif
	QDir userSettingsDirectory(userSettingsDirectoryName);
	if(!userSettingsDirectory.exists())
	{
		userSettingsDirectory.mkpath(userSettingsDirectoryName);
	}

	setPaths();
	loadSettings();

	QtSkin* skin=buildSkin(getSkinFilename());
	buildComponents(*skin);
	delete skin;

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(backgroundImage);
	setCentralWidget(backgroundImage);

	connect(this, SIGNAL(screenChanged()), backgroundImage, SLOT(updateScreen()));

	setWindowTitle(QApplication::translate("wp34s", "WP34s"));
	currentEmulator = this;

	loadMemory();
	startThreads();
}

QtEmulator::~QtEmulator()
{
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

void QtEmulator::buildComponents(const QtSkin& aSkin)
{
	screen=new QtScreen(aSkin);
	keyboard=new QtKeyboard(aSkin);

	QPixmap pixmap();

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

	QStringList skinSearchPath;
	QStringList imageSearchPath;
	QStringList memorySearchPath;


	skinSearchPath << userSettingsDirectoryName;
	imageSearchPath << userSettingsDirectoryName;
	memorySearchPath << userSettingsDirectoryName;

#ifdef Q_WS_MAC
	QString resourcesDir(applicationDir+RESOURCES_DIR);
	skinSearchPath << resourcesDir+SKIN_DIRECTORY;
	imageSearchPath << resourcesDir+IMAGE_DIRECTORY;
	memorySearchPath << resourcesDir+MEMORY_DIRECTORY;
#endif

	skinSearchPath << applicationDir+SKIN_DIRECTORY;
	imageSearchPath << applicationDir+IMAGE_DIRECTORY;
	memorySearchPath << applicationDir+MEMORY_DIRECTORY;

	QDir::setSearchPaths(SKIN_FILE_TYPE, skinSearchPath);
	QDir::setSearchPaths(IMAGE_FILE_TYPE, imageSearchPath);
	QDir::setSearchPaths(MEMORY_FILE_TYPE, memorySearchPath);
}

QString  QtEmulator::getSkinFilename()
{
	return "4 Medium 34s V3.xskin";
}

QtSkin* QtEmulator::buildSkin(const QString& aStringFilename)
{
	QFile skinFile(aStringFilename);
	return new QtSkin(skinFile);
}

void QtEmulator::loadSettings()
{
	 settings.beginGroup(WINDOWS_SETTINGS_GROUP);
	 move(settings.value(WINDOWS_POSITION_SETTING, QPoint(DEFAULT_POSITION_X, DEFAULT_POSITION_Y)).toPoint());
	 settings.endGroup();
}

void QtEmulator::saveSettings()
{
    settings.beginGroup(WINDOWS_SETTINGS_GROUP);
    settings.setValue(WINDOWS_POSITION_SETTING, pos());
    settings.endGroup();
}

void QtEmulator::loadMemory()
{
	QFile memoryFile(QString(MEMORY_FILE_TYPE)+':'+NON_VOLATILE_MEMORY_FILENAME);
	if(!memoryFile.exists() || !memoryFile.open(QIODevice::ReadOnly))
	{
		// TODO
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
	int reallyRead=dataStream.readRawData(get_filled_flash_region(aRegionIndex), memoryRegionSize);
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
	QFile memoryFile(getMemoryPath());
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
	QString regionPath(userSettingsDirectoryName+'/'+getRegionFileName(aRegionIndex));
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

QString QtEmulator::getMemoryPath() const
{
	return userSettingsDirectoryName+'/'+NON_VOLATILE_MEMORY_FILENAME;
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
	QFile memoryFile(getMemoryPath());
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

void QtEmulator::memoryWarning(const QString& aMessage, bool aResetFlag)
{
	QMessageBox messageBox;
	messageBox.setIcon(QMessageBox::Critical);
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
