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

#ifndef QTSPECIALDIGITPAINTER_H_
#define QTSPECIALDIGITPAINTER_H_

#include <QtGui>

class QtSpecialDigitPainter
{
public:
	QtSpecialDigitPainter(const QFont& aFont);

public:
	void paint(QPainter& aPainter, const QPoint& aPoint, int aMask);

private:
	int width;
	int height;
	int xOffset;
	int yOffset;
	int thickness;
};

#endif /* QTSPECIALDIGITPAINTER_H_ */
