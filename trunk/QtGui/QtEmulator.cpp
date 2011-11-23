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
	saveSetting();
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

}

void QtEmulator::saveSetting()
{

}

void QtEmulator::loadMemory()
{
	QSettings settings;
	QVariant variant=settings.value(NON_VOLATILE_MEMORY_SETTING);
	if(variant.isValid() && variant.canConvert(QVariant::ByteArray))
	{
		QByteArray memory=variant.toByteArray();
		if(memory.size()==get_memory_size())
		{
			memcpy(get_memory(), memory.constData(), memory.size());
		}
		else
		{
			// TODO
		}
	}
	else
	{
		// TODO
	}
}

void QtEmulator::saveMemory()
{
	prepare_memory_save();
	QSettings settings;
	QByteArray memory(get_memory(), get_memory_size());
	settings.setValue(NON_VOLATILE_MEMORY_SETTING, memory);
	settings.sync();
}
