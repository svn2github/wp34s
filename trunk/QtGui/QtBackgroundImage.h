/*
 * QtBackgroundImage.h
 *
 *  Created on: 20 nov. 2011
 *      Author: pascal
 */

#ifndef QTBACKGROUNDIMAGE_H_
#define QTBACKGROUNDIMAGE_H_

#include <QtGui>
#include "QtScreen.h"
#include "QtKeyboard.h"

class QtBackgroundImage: public QLabel
{
	Q_OBJECT

public:
	QtBackgroundImage(const QtSkin& aSkin, QtScreen& aScreen, QtKeyboard& aKeyboard);

public:
	QPixmap& getBackgroundPixmap();
	void setSkin(const QtSkin& aSkin);

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
	 QPixmap pixmap;
	 QtScreen& screen;
	 QtKeyboard& keyboard;
};

#endif /* QTBACKGROUNDIMAGE_H_ */
