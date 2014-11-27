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

#include <QMutexLocker>
#include "QtEmulator.h"
#include "QtKeyboard.h"
#include "QtEmulatorAdapter.h"

static const QtKeyCode INVALID_KEY_CODE;

QtKeyboard::QtKeyboard(const QtSkin& aSkin, bool anUseHShiftClick, bool anAlwaysUseHShiftClick, int anHShiftDelay, bool aShowToolTips)
	: keyboardBufferBegin(0),
	  keyboardBufferEnd(0),
	  currentKeyCode(INVALID_KEY_CODE),
	  useHShiftClick(anUseHShiftClick),
	  alwaysUseHShiftClick(anAlwaysUseHShiftClick),
	  hShiftDelay(anHShiftDelay),
	  showToolTips(aShowToolTips),
	  currentKeyHShifted(false),
	  autoRepeat(false),
	  fShiftLocked(false),
	  gShiftLocked(false),
	  hShiftLocked(false)
{
	setSkin(aSkin);
	hShiftTimer=new QTimer(this);
	hShiftTimer->setSingleShot(true);
	connect(hShiftTimer, SIGNAL(timeout()), this, SLOT(onHShift()));
	autoRepeatTimer=new QTimer(this);
	autoRepeatTimer->setSingleShot(true);
	connect(autoRepeatTimer, SIGNAL(timeout()), this, SLOT(onAutoRepeat()));
}

QtKeyboard::~QtKeyboard()
{
	for(QtKeyConstIterator keyIterator=keys.begin(); keyIterator!=keys.end(); ++keyIterator)
	{
		if(*keyIterator!=NULL)
		{
			delete *keyIterator;
		}
	}
}

void QtKeyboard::setSkin(const QtSkin& aSkin)
{
	hShiftHeight=aSkin.getHShiftHeight();
	keys=aSkin.getKeys();
	for(QtKeyConstIterator keyIterator=keys.constBegin(); keyIterator!=keys.constEnd(); ++keyIterator)
	{
		if(*keyIterator!=NULL)
		{
			keysByCode[(*keyIterator)->getCode()]=*keyIterator;
		}
	}
	catalogMenuKeys=aSkin.getCatalogMenuKeys();
}

bool QtKeyboard::processKeyPressedEvent(const QKeyEvent& aKeyEvent)
{
	if(isShowCatalogKey(aKeyEvent))
	{
		currentEmulator->showCatalogMenu();
	}
	else
	{
		autoRepeat=false;
		lastReleasedKeyCode=INVALID_KEY_CODE;
		QtKeyCode keyCode=findKeyCode(aKeyEvent);
		if(!aKeyEvent.isAutoRepeat() || isAutoRepeat(keyCode))
		{
			putKeyCode(keyCode);
			currentKeyCode=keyCode;
			emit keyPressed();
		}
	}
	return true;
}

bool QtKeyboard::processKeyReleasedEvent(const QKeyEvent& aKeyEvent)
{
	autoRepeat=false;
	lastReleasedKeyCode=INVALID_KEY_CODE;
	if(!aKeyEvent.isAutoRepeat())
	{
		QtKeyCode keyCode=findKeyCode(aKeyEvent);
		if(keyCode.getCode() == ON_CODE) {
			updateOnKeyTicks(false);
		}
		forward_key_released();
		currentKeyCode=INVALID_KEY_CODE;
	}
	return true;
}

bool QtKeyboard::processButtonPressedEvent(const QMouseEvent& aMouseEvent)
{
	autoRepeat=false;

	lastReleasedKeyCode=INVALID_KEY_CODE;
	if(aMouseEvent.button()==Qt::LeftButton)
	{
		QtKeyCode keyCode=findKeyCode(aMouseEvent.pos());
		return processKeyCodePressed(keyCode);
	}
	else
	{
		return false;
	}
}

bool QtKeyboard::processKeyCodePressed(const QtKeyCode& aKeyCode)
{
	putKeyCode(aKeyCode);
	currentKeyCode=aKeyCode;
	currentKeyHShifted=false;
	startHShiftTimer();
	if(isAutoRepeat(aKeyCode))
	{
		autoRepeat=true;
		startAutoRepeatTimer();
	}
	emit keyPressed();
	return currentKeyCode!=INVALID_KEY_CODE;
}

bool QtKeyboard::processButtonReleasedEvent(const QMouseEvent& aMouseEvent)
{
	Q_UNUSED(aMouseEvent)

	// Sometimes we received only button released events when pressing rapidly
	// this is a fix to avoid ignoring mouse clicks in such a case
	if(aMouseEvent.button()==Qt::LeftButton)
	{
		QtKeyCode keyCode=findKeyCode(aMouseEvent.pos());
		if(lastReleasedKeyCode.isValid() && lastReleasedKeyCode==keyCode)
		{
			processKeyCodePressed(lastReleasedKeyCode);
		}
		if(keyCode.getCode() == ON_CODE) {
			updateOnKeyTicks(false);
		}
	}

	autoRepeat=false;
	forward_key_released();
	lastReleasedKeyCode=currentKeyCode;
	currentKeyCode=INVALID_KEY_CODE;
	return true;
}

bool QtKeyboard::processMouseMovedEvent(const QMouseEvent& aMouseEvent)
{
	if(currentKeyCode.isValid())
	{
		QtKeyCode newKeyCode=findKeyCode(aMouseEvent.pos());
		if(newKeyCode.isValid() && newKeyCode!=currentKeyCode)
		{
			autoRepeat=false;
			lastReleasedKeyCode=INVALID_KEY_CODE;
		}
	}
	return true;
}

bool QtKeyboard::processDoubleClickEvent(const QMouseEvent& aMouseEvent)
{
	autoRepeat=false;
	QtKeyCode keyCode=findKeyCode(aMouseEvent.pos());
	if(setShifts(keyCode.getCode()))
	{
		emit keyPressed();
	}
	return true;
}

bool QtKeyboard::setShifts(int aCode)
{
	bool* shiftChanged=NULL;
	if(aCode==F_CODE)
	{
		shiftChanged=&fShiftLocked;
	}
	else if(aCode==G_CODE)
	{
		shiftChanged=&gShiftLocked;
	}
	else if(aCode==H_CODE)
	{
		shiftChanged=&hShiftLocked;
	}

	if(shiftChanged!=NULL)
	{
		bool newValue=!*shiftChanged;
		fShiftLocked=false;
		gShiftLocked=false;
		hShiftLocked=false;
		*shiftChanged=newValue;
		set_fshift_locked(fShiftLocked);
		set_gshift_locked(gShiftLocked);
		set_hshift_locked(hShiftLocked);
		return true;
	}
	else
	{
		return false;
	}
}

// Used to visually show that a key is hShifted when clicking on its green/H-Label
// but only after a while
void QtKeyboard::onHShift()
{
	currentKeyHShifted=currentKeyCode.isValid() && currentKeyCode.isHShifted();
	emit keyPressed();
}

void QtKeyboard::startAutoRepeatTimer()
{
	autoRepeatTimer->stop();
	if(autoRepeat)
	{
		autoRepeatTimer->start(AUTOREPEAT_FIRST_DELAY);
	}
}

void QtKeyboard::showCatalogMenu()
{
	currentKeyCode=INVALID_KEY_CODE;
}

void QtKeyboard::onAutoRepeat()
{
	autoRepeatTimer->stop();
	if(autoRepeat && currentKeyCode.isValid() && (isAutoRepeat(currentKeyCode)))
	{
		putKeyCode(currentKeyCode);
		emit keyPressed();
		autoRepeatTimer->start(AUTOREPEAT_DELAY);
	}
	else
	{
		autoRepeat=false;
	}
}

bool QtKeyboard::isAutoRepeat(const QtKeyCode& aKeyCode) const
{
	return aKeyCode.getCode()==uparrow_code() || (aKeyCode.getCode()==downarrow_code() && (!is_runmode() || is_catalogue_mode()));
}

bool QtKeyboard::isUseHShiftClick()
{
	return useHShiftClick;
}

void QtKeyboard::setUseHShiftClick(bool anUseHShiftClick)
{
	useHShiftClick=anUseHShiftClick;
}

bool QtKeyboard::isAlwaysUseHShiftClick()
{
	return alwaysUseHShiftClick;
}

void QtKeyboard::setAlwaysUseHShiftClick(bool anAlwaysUseHShiftClick)
{
	alwaysUseHShiftClick=anAlwaysUseHShiftClick;
}

bool QtKeyboard::isShowToolTips()
{
	return showToolTips;
}

void QtKeyboard::setShowToolTips(bool aShowToolTips)
{
	showToolTips=aShowToolTips;
}

int QtKeyboard::getHShiftDelay()
{
	return hShiftDelay;
}

void QtKeyboard::setHShiftDelay(int anHShiftDelay)
{
	hShiftDelay=anHShiftDelay;
}

int QtKeyboard::getKey()
{
	QMutexLocker mutexLocker(&mutex);
	return getKeyNoLock();
}

int QtKeyboard::getKeyNoLock()
{
	if(keyboardBufferBegin==keyboardBufferEnd)
	{
		return -1;
	}
	else
	{
		int key=keyboardBuffer[keyboardBufferBegin];
		keyboardBufferBegin=(keyboardBufferBegin+1)%KEYBOARD_BUFFER_SIZE;
		return key;
	}
}

void QtKeyboard::putKeyCode(const QtKeyCode& aKeyCode)
{
	currentKeyHShifted=false;
	if(aKeyCode.isValid())
	{
		if(!hShiftLocked && !is_hshifted() && aKeyCode.isHShifted())
		{
			currentKeyHShifted=true;
			putKey(H_CODE);
		}
		putKey(aKeyCode.getCode());
	}
}

void QtKeyboard::putKey(char aKey)
{
	if(aKey == ON_CODE) {
		updateOnKeyTicks(true);
	}
	QMutexLocker mutexLocker(&mutex);
	keyboardBuffer[keyboardBufferEnd]=aKey;
	keyboardBufferEnd=(keyboardBufferEnd+1)%KEYBOARD_BUFFER_SIZE;
	keyWaitCondition.wakeAll();
}

void QtKeyboard::putKeyIfBufferEmpty(char aKey)
{
	QMutexLocker mutexLocker(&mutex);
	if(!isKeyPressedNoLock())
	{
		keyboardBuffer[keyboardBufferEnd]=aKey;
		keyboardBufferEnd=(keyboardBufferEnd+1)%KEYBOARD_BUFFER_SIZE;
		keyWaitCondition.wakeAll();
	}
}

bool QtKeyboard::isKeyPressed()
{
	QMutexLocker mutexLocker(&mutex);
	return isKeyPressedNoLock();
}

bool QtKeyboard::isKeyPressedNoLock()
{
	return keyboardBufferBegin!=keyboardBufferEnd;
}

int QtKeyboard::waitKey()
{
	QMutexLocker mutexLocker(&mutex);
	if(!isKeyPressedNoLock())
	{
		keyWaitCondition.wait(&mutex);
	}
	return getKeyNoLock();
}

static int keyEventToKeycode(const QKeyEvent&);

bool QtKeyboard::isShowCatalogKey(const QKeyEvent& aKeyEvent) const
{
	int keyCode=keyEventToKeycode(aKeyEvent);
	if(keyCode<0)
	{
		return false;
	}
    QKeySequence sequence(keyCode);
	for(KeySequenceConstIterator sequenceIterator=catalogMenuKeys.begin(); sequenceIterator!=catalogMenuKeys.end(); ++sequenceIterator)
	{
		if(sequence.matches(*sequenceIterator)==QKeySequence::ExactMatch)
		{
			return true;
		}
	}
	return false;
}

// This is not very efficient but it works and because keys are only 40, with just a few sequences each, this will do
QtKeyCode QtKeyboard::findKeyCode(const QKeyEvent& aKeyEvent) const
{
	int keyCode=keyEventToKeycode(aKeyEvent);
	if(keyCode<0)
	{
		return -1;
	}

    QKeySequence sequence(keyCode);
	for(QtKeyConstIterator keyIterator=keys.begin(); keyIterator!=keys.end(); ++keyIterator)
	{
		if(*keyIterator!=NULL)
		{
			for(KeySequenceConstIterator sequenceIterator=(*keyIterator)->getKeySequences().begin(); sequenceIterator!=(*keyIterator)->getKeySequences().end(); ++sequenceIterator)
			{
				if(sequence.matches(*sequenceIterator)==QKeySequence::ExactMatch)
				{
					return QtKeyCode((*keyIterator)->getCode());
				}
			}
		}
	}
	return INVALID_KEY_CODE;
}

// Not very efficient too but with 40 keys, probably fast enough and avoid the trouble of building sorted trees
QtKeyCode QtKeyboard::findKeyCode(const QPoint& aPoint) const
{
	for(QtKeyConstIterator keyIterator=keys.begin(); keyIterator!=keys.end(); ++keyIterator)
	{
		if(*keyIterator!=NULL)
		{
			QRect keyRect=(*keyIterator)->getRectangle();
			if(keyRect.contains(aPoint))
			{
				bool hShifted=false;
				int code=(*keyIterator)->getCode();
				if(code!=F_CODE && code!=G_CODE && code!=H_CODE && useHShiftClick && (alwaysUseHShiftClick || is_not_shifted()))
				{
					QRect hRect(keyRect);
					hRect.setTop(keyRect.bottom()-hShiftHeight);
					hShifted=hRect.contains(aPoint);
				}
				return QtKeyCode(code, hShifted);
			}
		}
	}
	return INVALID_KEY_CODE;
}

const QtKey* QtKeyboard::findKey(const QtKeyCode& aKeyCode) const
{
	if(aKeyCode.isValid())
	{
		return keysByCode.value(aKeyCode.getCode(), NULL);
	}
	else
	{
		return NULL;
	}
}

void QtKeyboard::startHShiftTimer()
{
	hShiftTimer->stop();
	if(!hShiftLocked && currentKeyCode.isHShifted())
	{
		hShiftTimer->start(hShiftDelay);
	}
}

void QtKeyboard::paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent)
{
	Q_UNUSED(aPaintEvent);

	if(fShiftLocked)
	{
		invertKeycode(F_CODE, aBackgroundImage);
	}
	else if(gShiftLocked)
	{
		invertKeycode(G_CODE, aBackgroundImage);
	}
	else if(hShiftLocked)
	{
		invertKeycode(H_CODE, aBackgroundImage);
	}

	const QtKey* key=findKey(currentKeyCode);
	if(key!=NULL && key->getRectangle().isValid())
	{
		invertKey(key, aBackgroundImage);
		if(currentKeyHShifted && !hShiftLocked)
		{
			invertKeycode(H_CODE, aBackgroundImage);
		}
	}
}

void QtKeyboard::invertKeycode(int aCode, QtBackgroundImage& aBackgroundImage)
{
	const QtKey* Key=findKey(aCode);
	invertKey(Key, aBackgroundImage);
}

void QtKeyboard::invertKey(const QtKey* aKey, QtBackgroundImage& aBackgroundImage)
{
	// There are simpler way to invert video but this one works on every platform tested yet
	QRect keyRectangle=aKey->getRectangle();
	QPixmap keyPixmap=aBackgroundImage.getBackgroundPixmap().copy(keyRectangle);
	QImage keyImage=keyPixmap.toImage();
	keyImage.invertPixels();
	QPainter painter(&aBackgroundImage);
	painter.drawImage(keyRectangle, keyImage);
	aBackgroundImage.update(aKey->getRectangle());
}

static int keyEventToKeycode(const QKeyEvent& keyEvent)
{
	int keyInt = keyEvent.key();
	Qt::Key key = static_cast<Qt::Key>(keyInt);

	if(key == 0 ||
		key == Qt::Key_unknown ||
		key == Qt::Key_Control ||
		key == Qt::Key_Shift ||
		key == Qt::Key_Alt ||
		key == Qt::Key_Meta)
	{
		return -1;
	}

	Qt::KeyboardModifiers modifiers = keyEvent.modifiers();
	if(modifiers & Qt::ShiftModifier)
	{
		keyInt += Qt::SHIFT;
	}
	if(modifiers & Qt::ControlModifier)
	{
		keyInt += Qt::CTRL;
	}
	if(modifiers & Qt::AltModifier)
	{
		keyInt += Qt::ALT;
	}
	if(modifiers & Qt::MetaModifier)
	{
		keyInt += Qt::META;
	}
	return keyInt;
}

extern "C"
{

int is_key_pressed_adapter()
{
	return currentEmulator->getKeyboard().isKeyPressed();
}

int put_key_adapter(int key)
{
	currentEmulator->getKeyboard().putKey(key);
	return key;
}

void add_heartbeat_adapter(int key)
{
	currentEmulator->getKeyboard().putKeyIfBufferEmpty(key);
}

void increment_on_key_ticks_adapter()
{
	currentEmulator->getKeyboard().incrementOnKeyTicks();
}

int get_key(void) // Used by xeq() to empty the keyboard buffer
{
	return currentEmulator->getKeyboard().getKey();
}

// These are defined in main.c but the Qt build doesn't use that file
volatile char OnKeyPressed;
volatile unsigned int OnKeyTicks;

}

void QtKeyboard::incrementOnKeyTicks()
{
	QMutexLocker mutexLocker(&mutex);
	if (OnKeyPressed) {
		++OnKeyTicks;
	}
}

void QtKeyboard::updateOnKeyTicks(bool pressed)
{
	QMutexLocker mutexLocker(&mutex);
	if(pressed) {
		OnKeyPressed = 1;
	}
	else
	{
		OnKeyPressed = 0;
		OnKeyTicks = 0;
	}
}
