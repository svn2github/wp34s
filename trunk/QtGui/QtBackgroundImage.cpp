/*
 * QtBackgroundImage.cpp
 *
 *  Created on: 20 nov. 2011
 *      Author: pascal
 */

#include "QtBackgroundImage.h"
#include "QtEmulator.h"

class BackgroundImageEventFilter: public QObject
{
public:
	BackgroundImageEventFilter(QtKeyboard& aKeyboard)
	: keyboard(aKeyboard)
	{
	}

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
	QtKeyboard& keyboard;
};

bool BackgroundImageEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
		return keyboard.processButtonPressedEvent(*mouseEvent);
	}
	else if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
		return keyboard.processButtonReleasedEvent(*mouseEvent);
	}
	else if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		return keyboard.processKeyPressedEvent(*keyEvent);
	}
	else if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		return keyboard.processKeyReleasedEvent(*keyEvent);
	}
	else
	{
		return QObject::eventFilter(obj, event);
	}
}

QtBackgroundImage::QtBackgroundImage(const QtSkin& aSkin, QtScreen& aScreen, QtKeyboard& aKeyboard)
	: screen(aScreen), keyboard(aKeyboard)
{
	setSkin(aSkin);
	setPixmap(pixmap);
	installEventFilter(new BackgroundImageEventFilter(aKeyboard));
	setFocusPolicy(Qt::StrongFocus);
	setFixedSize(pixmap.size());
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
