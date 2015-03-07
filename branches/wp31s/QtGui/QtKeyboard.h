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
#define F_CODE 30
#define ON_CODE 36


#define DEFAULT_USE_FSHIFT_CLICK true
#define DEFAULT_ALWAYS_USE_FSHIFT_CLICK false
#define DEFAULT_FSHIFT_DELAY 200
#define DEFAULT_DISPLAY_AS_STACK false

#define AUTOREPEAT_FIRST_DELAY 500
#define AUTOREPEAT_DELAY 100

class QtKeyboard: public QObject
{
	Q_OBJECT

public:
	QtKeyboard(const QtSkin& aSkin, bool anUseFShiftClick, bool anAlwaysUseFShiftClick, int anFShiftDelay, bool aShowToolTips);
	virtual ~QtKeyboard();

public:
	void setSkin(const QtSkin& aSkin);
	bool processKeyPressedEvent(const QKeyEvent& aKeyEvent);
	bool processKeyReleasedEvent(const QKeyEvent& aKeyEvent);
	bool processButtonPressedEvent(const QMouseEvent& aMouseEvent);
	bool processButtonReleasedEvent(const QMouseEvent& aMouseEvent);
	bool processMouseMovedEvent(const QMouseEvent& aMouseEvent);
	bool processDoubleClickEvent(const QMouseEvent& aMouseEvent);
	bool processKeyCodePressed(const QtKeyCode& aKeyCode);
	bool setShifts(int aCode);
	int getKey();
	void putKeyCode(const QtKeyCode& aKeyCode);
	void putKey(char aKey);
	void putKeyIfBufferEmpty(char aKey);
	bool isKeyPressed();
	int waitKey();
	void paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent);
	void invertKeycode(int aCode, QtBackgroundImage& aBackgroundImage);
	void invertKey(const QtKey* aKey, QtBackgroundImage& aBackgroundImage);
	bool isUseFShiftClick();
	void setUseFShiftClick(bool anUseFShiftClick);
	bool isAlwaysUseFShiftClick();
	void setAlwaysUseFShiftClick(bool anAlwaysUseFShiftClick);
	int getFShiftDelay();
	void setFShiftDelay(int anFShiftDelay);
	bool isShowToolTips();
	void setShowToolTips(bool aShowToolTips);
	QtKeyCode findKeyCode(const QKeyEvent& aKeyEvent) const;
	void showCatalogMenu();
	void incrementOnKeyTicks();

private slots:
	void onFShift();
	void onAutoRepeat();

signals:
	void keyPressed();

private:
	bool isKeyPressedNoLock();
	int getKeyNoLock();
	bool isShowCatalogKey(const QKeyEvent& aKeyEvent) const;
	QtKeyCode findKeyCode(const QPoint& aPoint) const;
    const QtKey* findKey(const QtKeyCode& aKeyCode) const;
    void startFShiftTimer();
    void startAutoRepeatTimer();
    bool isAutoRepeat(const QtKeyCode& aKeyCode) const;
	void updateOnKeyTicks(bool pressed);

private:
    int fShiftHeight;
    QtKeyList keys;
    KeySequenceList catalogMenuKeys;
    QMutex mutex;
    QWaitCondition keyWaitCondition;
    char keyboardBuffer[KEYBOARD_BUFFER_SIZE];
    volatile int keyboardBufferBegin, keyboardBufferEnd;
    QtKeyCode currentKeyCode;
    QtKeyCode lastReleasedKeyCode;
    bool useFShiftClick;
    bool alwaysUseFShiftClick;
    int fShiftDelay;
    bool showToolTips;
    bool currentKeyFShifted;
    bool autoRepeat;
    bool fShiftLocked;
    QTimer* fShiftTimer;
    QTimer* autoRepeatTimer;
    QHash<int, const QtKey*> keysByCode;
};

#endif /* QTKEYBOARD_H_ */
