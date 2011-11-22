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

#include <QPainter>
#include "QtScreenPainter.h"

QtScreenPainter::QtScreenPainter()
{
}

QtScreenPainter::~QtScreenPainter()
{
}

PolygonPainter::PolygonPainter(const QPolygon& aPolygon)
	: polygon(aPolygon)
{
}

void PolygonPainter::paint(QPixmap& aPixmap, QPainter& aPainter)
{
	Q_UNUSED(aPixmap)

	aPainter.drawPolygon(polygon);
}


CopyPainter::CopyPainter(const QRect& aSource, const QPoint& aDestination)
	: source(aSource), destination(aDestination)
{
}

void CopyPainter::paint(QPixmap& aPixmap, QPainter& aPainter)
{
	QPixmap copy=aPixmap.copy(source);
	aPainter.drawPixmap(destination, copy);
}

DotPainter::DotPainter()
{
}

void DotPainter::paint(QPixmap& aPixmap, QPainter& aPainter)
{
	for(LCDPainterListIterator painterIterator=lcdPainters.begin(); painterIterator!=lcdPainters.end(); ++painterIterator)
	{
		(*painterIterator)->paint(aPixmap, aPainter);
	}
}

void DotPainter::addLCDPainter(QtScreenPainter* aLCDPainter)
{
	if(aLCDPainter!=NULL)
	{
		lcdPainters.append(aLCDPainter);
	}
}
