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

#ifndef QTSCREEN_H_
#define QTSCREEN_H_

#include <QPaintDevice>
#include <QPaintEvent>
#include "QtSkin.h"

// We need to forward define it as we are included by QtBackgroundImage.h
class QtBackgroundImage;

class QtScreen
{
public:
	QtScreen(const QtSkin& aSkin);
	~QtScreen();

public:
	const QRect& getScreenRectangle() const;
	void paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent);

private:
	QRect screenRectangle;
    QColor screenForeground;
    QColor screenBackground;
    DotPainterList dotPainters;
};

extern "C"
{
	extern void updateScreen();
}

#endif /* QTSCREEN_H_ */
