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

QtKeyboard::QtKeyboard(const QtSkin& aSkin)
	: keyboardBufferBegin(0), keyboardBufferEnd(0), currentKeyCode(INVALID_KEY_CODE), currentKeyHShifted(false)
{
	setSkin(aSkin);
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
}

bool QtKeyboard::processKeyPressedEvent(const QKeyEvent& aKeyEvent)
{
	QtKeyCode keyCode=findKeyCode(aKeyEvent);
	putKeyCode(keyCode);
	currentKeyCode=keyCode;
	emit keyPressed();
	return true;
}

bool QtKeyboard::processKeyReleasedEvent(const QKeyEvent& aKeyEvent)
{
	Q_UNUSED(aKeyEvent)

	forward_key_released();
	currentKeyCode=INVALID_KEY_CODE;
	return true;
}

bool QtKeyboard::processButtonPressedEvent(const QMouseEvent& aMouseEvent)
{
	currentKeyCode=findKeyCode(aMouseEvent.pos());
	emit keyPressed();
	return true;
}

bool QtKeyboard::processButtonReleasedEvent(const QMouseEvent& aMouseEvent)
{
	Q_UNUSED(aMouseEvent)

	putKeyCode(findKeyCode(aMouseEvent.pos()));
	forward_key_released();
	currentKeyCode=INVALID_KEY_CODE;
	return true;
}

bool QtKeyboard::processMouseMovedEvent(const QMouseEvent& aMouseEvent)
{
	if(currentKeyCode.isValid())
	{
		QtKeyCode newKeyCode=findKeyCode(aMouseEvent.pos());
		if(newKeyCode.isValid())
		{
			currentKeyCode=newKeyCode;
			currentKeyHShifted=currentKeyCode.isHShifted();
			emit keyPressed();
		}
	}
	return true;
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
		if(aKeyCode.isHShifted())
		{
			currentKeyHShifted=true;
			putKey(H_CODE);
		}
		putKey(aKeyCode.getCode());
	}
}

void QtKeyboard::putKey(char aKey)
{
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
				if(code!=F_CODE && code!=G_CODE && code!=H_CODE)
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

void QtKeyboard::paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent)
{
	Q_UNUSED(aPaintEvent);

	const QtKey* key=findKey(currentKeyCode);
	if(key!=NULL && key->getRectangle().isValid())
	{
		invert(key, aBackgroundImage);
		if(currentKeyHShifted)
		{
			const QtKey* hKey=findKey(H_CODE);
			invert(hKey, aBackgroundImage);
		}
	}
}

void QtKeyboard::invert(const QtKey* aKey, QtBackgroundImage& aBackgroundImage)
{
	// There are simple way to invert video but this one works on every platform tested yet
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

}

