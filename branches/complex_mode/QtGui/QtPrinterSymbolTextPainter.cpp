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

#include "QtPrinterSymbolTextPainter.h"
#include "QtEmulator.h"
#include <QtGui>

#define IMAGE_NAME "printer.png"
#define PROTOTYPE_TEXT "W"

#define SIZE_FACTOR 0.8f

void QtPrinterSymbolTextPainter::paint(const QPoint& aPoint, QPainter& aPainter, const QFont& aFontLower)
{
	Q_UNUSED(aFontLower)

	QPixmap* pixmap=pixmaps.value(aPainter.font().key());
	if(pixmap==NULL)
	{
		QPixmap fullResPixmap;
		fullResPixmap.load(QString(IMAGE_FILE_TYPE)+':'+ IMAGE_NAME);
		delete pixmap;
		QSize fullSize=aPainter.fontMetrics().boundingRect(PROTOTYPE_TEXT).size();
		QSize realSize(fullSize.width()*SIZE_FACTOR, fullSize.height()*SIZE_FACTOR);
		deltaX=(fullSize.width()-realSize.width())/2;
		deltaY=(fullSize.height()-realSize.height())/2;
		pixmap = new QPixmap(fullResPixmap.scaled(realSize));
		pixmaps.insert(aPainter.font().key(), pixmap);
	}
	aPainter.drawPixmap(QPoint(aPoint.x()+deltaX, aPoint.y()+deltaY), *pixmap);
}

int QtPrinterSymbolTextPainter::width(QPainter& aPainter, const QFont& aFontLower)
{
	Q_UNUSED(aFontLower)

	return aPainter.fontMetrics().width(PROTOTYPE_TEXT);
}

