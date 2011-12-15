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
#include "QtKey.h"
#include "QtKeyCode.h"

// We need to forward define it as we are included by QtBackgroundImage.h
class QtBackgroundImage;

#define KEYBOARD_BUFFER_SIZE 16
#define F_CODE 9
#define G_CODE 10
#define H_CODE 11

class QtKeyboard: public QObject
{
	Q_OBJECT

public:
	QtKeyboard(const QtSkin& aSkin);
	virtual ~QtKeyboard();

public:
	void setSkin(const QtSkin& aSkin);
	bool processKeyPressedEvent(const QKeyEvent& aKeyEvent);
	bool processKeyReleasedEvent(const QKeyEvent& aKeyEvent);
	bool processButtonPressedEvent(const QMouseEvent& aMouseEvent);
	bool processButtonReleasedEvent(const QMouseEvent& aMouseEvent);
	bool processMouseMovedEvent(const QMouseEvent& aMouseEvent);
	int getKey();
	void putKeyCode(const QtKeyCode& aKeyCode);
	void putKey(char aKey);
	void putKeyIfBufferEmpty(char aKey);
	bool isKeyPressed();
	int waitKey();
	void paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent);
	void invert(const QtKey* aKey, QtBackgroundImage& aBackgroundImage);

signals:
	void keyPressed();

private:
	bool isKeyPressedNoLock();
	int getKeyNoLock();
	QtKeyCode findKeyCode(const QKeyEvent& aKeyEvent) const;
	QtKeyCode findKeyCode(const QPoint& aPoint) const;
    const QtKey* findKey(const QtKeyCode& aKeyCode) const;

private:
    int hShiftHeight;
    QtKeyList keys;
    QMutex mutex;
    QWaitCondition keyWaitCondition;
    char keyboardBuffer[KEYBOARD_BUFFER_SIZE];
    volatile int keyboardBufferBegin, keyboardBufferEnd;
    QtKeyCode currentKeyCode;
    bool currentKeyHShifted;
    QHash<int, const QtKey*> keysByCode;
};

#endif /* QTKEYBOARD_H_ */
