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

#include "QtSpecialDigitPainter.h"

#define PROTOTYPE_CHAR '8'
#define THICKNESS_FACTOR 0.2f
#define X_OFFSET_FACTOR 0.1f
#define Y_OFFSET_FACTOR 0.1f

#define D_TOP 64
#define D_TL 32
#define D_TR 8
#define D_MIDDLE 16
#define D_BL 4
#define D_BR 1
#define D_BOTTOM 2

QtSpecialDigitPainter::QtSpecialDigitPainter(const QFont& aFont)
{
	QFontMetrics fontMetrics(aFont);
	width=fontMetrics.width(PROTOTYPE_CHAR);
	height=fontMetrics.boundingRect(PROTOTYPE_CHAR).height();
	xOffset=(int) (width*X_OFFSET_FACTOR);
	yOffset=(int) (height*Y_OFFSET_FACTOR);
	width-=xOffset*2;
	height-=yOffset*2;
	thickness=(int) (width*THICKNESS_FACTOR);
}

void QtSpecialDigitPainter::paint(QPainter& aPainter, const QPoint& aPoint, int aMask)
{
	aPainter.save();
	QPen pen(aPainter.brush(), thickness);
	aPainter.setPen(pen);
	QPoint point(aPoint.x()+xOffset, aPoint.y()+yOffset);
	if(aMask & D_TOP) aPainter.drawLine(point.x(), point.y(), point.x()+width, point.y());
	if(aMask & D_TL) aPainter.drawLine(point.x(), point.y(), point.x(), point.y()+height/2);
	if(aMask & D_TR) aPainter.drawLine(point.x()+width, point.y(), point.x()+width, point.y()+height/2);
	if(aMask & D_MIDDLE) aPainter.drawLine(point.x(), point.y()+height/2, point.x()+width, point.y()+height/2);
	if(aMask & D_BL) aPainter.drawLine(point.x(), point.y()+height/2, point.x(), point.y()+height);
	if(aMask & D_BR) aPainter.drawLine(point.x()+width, point.y()+height/2, point.x()+width, point.y()+height);
	if(aMask & D_BOTTOM) aPainter.drawLine(point.x(), point.y()+height, point.x()+width, point.y()+height);
	aPainter.restore();
}

