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

#ifndef QTBACKGROUNDIMAGE_H_
#define QTBACKGROUNDIMAGE_H_

#include <QtGui>
#include "QtScreen.h"
#include "QtKeyboard.h"

#define MOVE_MARGIN_X 10
#define MOVE_MARGIN_Y 10

class QtBackgroundImage: public QLabel
{
	Q_OBJECT

public:
	QtBackgroundImage(const QtSkin& aSkin, QtScreen& aScreen, QtKeyboard& aKeyboard, QWidget* aParent=0);

public:
	QPixmap& getBackgroundPixmap();
	void setSkin(const QtSkin& aSkin);
	void showToolTips(bool aShowToolTipsFlag);

// Overriden event-handling methods
public:
	void keyPressEvent(QKeyEvent* aKeyEvent);
	void keyReleaseEvent(QKeyEvent* aKeyEvent);
	void mousePressEvent(QMouseEvent* aMouseEvent);
	void mouseReleaseEvent(QMouseEvent* aMouseEvent);
	void mouseMoveEvent(QMouseEvent* aMouseEvent);
	void mouseDoubleClickEvent(QMouseEvent* aMouseEvent);

public slots:
	void updateScreen();

protected:
	 void paintEvent(QPaintEvent *);

private:
	 void moveWindow(QMouseEvent* aMouseEvent);
	 void addToolTip(const QtKey& aKey);
	 void clearToolTips();

private:
	 QPixmap pixmap;
	 QRect textRect;
	 QtScreen& screen;
	 QtKeyboard& keyboard;
	 bool dragging;
	 QPoint lastDragPosition;
	 QList<QLabel*> tooltipLabels;
};

#endif /* QTBACKGROUNDIMAGE_H_ */
