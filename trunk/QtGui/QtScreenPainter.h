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

#ifndef QTSCREENPAINTER_H_
#define QTSCREENPAINTER_H_

#include <QImage>
#include <QLinkedList>

class QtScreenPainter
{
public:
	QtScreenPainter();
	virtual ~QtScreenPainter();
	virtual void paint(QPixmap& aPixmap, QPainter& aPainter)=0;
};

class PolygonPainter: public QtScreenPainter
{
public:
	PolygonPainter(const QPolygon& aPolygon);
	void paint(QPixmap& aPixmap, QPainter& aPainter);

private:
	QPolygon polygon;
};

class CopyPainter: public QtScreenPainter
{
public:
	CopyPainter(const QRect& aSource, const QPoint& aDestination);
	void paint(QPixmap& aPixmap, QPainter& aPainter);

private:
	QRect source;
	QPoint destination;
};

typedef QLinkedList<QtScreenPainter*> LCDPainterList;
typedef LCDPainterList::iterator LCDPainterListIterator;

class DotPainter: public QtScreenPainter
{
public:
	DotPainter();
	void paint(QPixmap& aPixmap, QPainter& aPainter);
	void addLCDPainter(QtScreenPainter* aLCDPainter);

private:
	LCDPainterList lcdPainters;
};

#endif /* LCDPAINTER_H_ */
