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

#ifndef QTKEYBOARD_H_
#define QTKEYBOARD_H_

#include <QtGui>
#include <QMutex>
#include <QWaitCondition>
#include "QtSkin.h"

#define KEYBOARD_BUFFER_SIZE 16

class QtKeyboard
{
public:
	QtKeyboard(const QtSkin& aSkin);
	~QtKeyboard();

public:
	bool processKeyPressedEvent(const QKeyEvent& aKeyEvent);
	bool processKeyReleasedEvent(const QKeyEvent& aKeyEvent);
	bool processButtonPressedEvent(const QMouseEvent& aMouseEvent);
	bool processButtonReleasedEvent(const QMouseEvent& aMouseEvent);
	int getKey();
	void putKey(char aKey);
	void putKeyIfBufferEmpty(char aKey);
	bool isKeyPressed();
	int waitKey();

private:
	bool isKeyPressedNoLock();
	int getKeyNoLock();
    int findKey(const QKeyEvent& aKeyEvent) const;
    int findKey(const QPoint& aPoint) const;

private:
    QtKeyList keys;
    QMutex mutex;
    QWaitCondition keyWaitCondition;
    char keyboardBuffer[KEYBOARD_BUFFER_SIZE];
    int keyboardBufferBegin, keyboardBufferEnd;
};

#endif /* QTKEYBOARD_H_ */
