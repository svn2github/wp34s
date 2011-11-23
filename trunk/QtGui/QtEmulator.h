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
#define WINDOWS_POSITION_SETTING "Position"
#define NON_VOLATILE_MEMORY_SETTING "NonVolatileMemory"

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

private:
     QString getSkinFilename();
     QtSkin* buildSkin(const QString& aStringFilename);
     void buildComponents(const QtSkin& aSkin);
     void startThreads();
     void loadSettings();
     void saveSetting();
     void loadMemory();
     void saveMemory();

private:
     QtKeyboard* keyboard;
     QtScreen* screen;
     QtBackgroundImage* backgroundImage;
     QtCalculatorThread* calculatorThread;
     QtHeartBeatThread* heartBeatThread;

signals:
	void screenChanged();
};

extern QtEmulator* currentEmulator;

#endif /* QTEMULATOR_H_ */
