/*
 * QtBackgroundImage.cpp
 *
 *  Created on: 20 nov. 2011
 *      Author: pascal
 */

#include "QtBackgroundImage.h"
#include "QtEmulator.h"


QtBackgroundImage::QtBackgroundImage(const QtSkin& aSkin, QtScreen& aScreen, QtKeyboard& aKeyboard)
	: screen(aScreen), keyboard(aKeyboard)
{
	setSkin(aSkin);
	setPixmap(pixmap);
	setFocusPolicy(Qt::StrongFocus);
	setFixedSize(pixmap.size());
	connect(&aKeyboard, SIGNAL(keyPressed()), this, SLOT(updateScreen()));
}

void QtBackgroundImage::setSkin(const QtSkin& aSkin)
{
	if(!pixmap.load(QString(IMAGE_FILE_TYPE)+':'+aSkin.getPictureName()))
	{
		throw *(new QtSkinException(QString("Cannot find picture ")+aSkin.getPictureName()));
	}
	setPixmap(pixmap);
	setFixedSize(pixmap.size());
}

QPixmap& QtBackgroundImage::getBackgroundPixmap()
{
	return pixmap;
}

void QtBackgroundImage::keyPressEvent(QKeyEvent* aKeyEvent)
{
	keyboard.processKeyPressedEvent(*aKeyEvent);
}

void QtBackgroundImage::keyReleaseEvent(QKeyEvent* aKeyEvent)
{
	keyboard.processKeyReleasedEvent(*aKeyEvent);
}

void QtBackgroundImage::mousePressEvent(QMouseEvent* aMouseEvent)
{
	keyboard.processButtonPressedEvent(*aMouseEvent);
}

void QtBackgroundImage::mouseReleaseEvent(QMouseEvent* aMouseEvent)
{
	keyboard.processButtonReleasedEvent(*aMouseEvent);
}

void QtBackgroundImage::mouseMoveEvent(QMouseEvent* aMouseEvent)
{
	keyboard.processMouseMovedEvent(*aMouseEvent);
}

void QtBackgroundImage::mouseDoubleClickEvent(QMouseEvent* aMouseEvent)
{
	keyboard.processDoubleClickEvent(*aMouseEvent);
}

void QtBackgroundImage::paintEvent(QPaintEvent* aPaintEvent)
{
	QLabel::paintEvent(aPaintEvent);
	screen.paint(*this, *aPaintEvent);
	keyboard.paint(*this, *aPaintEvent);
}

void QtBackgroundImage::updateScreen()
{
	update(screen.getScreenRectangle());
}
