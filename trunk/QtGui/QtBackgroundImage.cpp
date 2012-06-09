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

#include "QtBackgroundImage.h"
#include "QtEmulator.h"

static QPoint MOVE_MARGIN(MOVE_MARGIN_X, MOVE_MARGIN_Y);
static QPoint MOVE_OTHER_MARGIN(MOVE_MARGIN_X, -MOVE_MARGIN_Y);

QtBackgroundImage::QtBackgroundImage(const QtSkin& aSkin, QtScreen& aScreen, QtKeyboard& aKeyboard, QWidget* aParent)
	: QLabel(aParent), screen(aScreen), keyboard(aKeyboard), dragging(false)
{
	setSkin(aSkin);
	setPixmap(pixmap);
	setMask(pixmap.mask());
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
	setMask(pixmap.mask());
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
	if(!keyboard.processButtonPressedEvent(*aMouseEvent) && aMouseEvent->button()==Qt::LeftButton)
	{
		dragging=true;
		lastDragPosition=aMouseEvent->globalPos();
	}
}

void QtBackgroundImage::mouseReleaseEvent(QMouseEvent* aMouseEvent)
{
	if(!dragging)
	{
		keyboard.processButtonReleasedEvent(*aMouseEvent);
	}
	dragging=false;
}

void QtBackgroundImage::mouseMoveEvent(QMouseEvent* aMouseEvent)
{
	if(!dragging)
	{
		keyboard.processMouseMovedEvent(*aMouseEvent);
	}
	else
	{
		moveWindow(aMouseEvent);
	}
}

void QtBackgroundImage::moveWindow(QMouseEvent* aMouseEvent)
{
	QWidget* parentWindow=window();
	QPoint offset=aMouseEvent->globalPos()-lastDragPosition;
	QPoint topLeft=parentWindow->pos()+parentWindow->rect().topLeft()+MOVE_MARGIN+offset;
	QPoint topRight=parentWindow->pos()+parentWindow->rect().topRight()-MOVE_OTHER_MARGIN+offset;
	QPoint bottomLeft=parentWindow->pos()+parentWindow->rect().bottomLeft()+MOVE_OTHER_MARGIN+offset;
	QPoint bottomRight=parentWindow->pos()+parentWindow->rect().bottomRight()-MOVE_MARGIN+offset;
	QDesktopWidget* desktopWidget=QApplication::desktop();
	if(desktopWidget->availableGeometry(topLeft).contains(topLeft)
			|| desktopWidget->availableGeometry(topRight).contains(topRight)
			|| desktopWidget->availableGeometry(bottomLeft).contains(bottomLeft)
			|| desktopWidget->availableGeometry(bottomRight).contains(bottomRight))
	{
		parentWindow->move(parentWindow->pos()+offset);
	}
	lastDragPosition=aMouseEvent->globalPos();
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
