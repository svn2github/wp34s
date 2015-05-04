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

#include "QtNormalTextPainter.h"

QtNormalTextPainter::QtNormalTextPainter(QChar aChar) :
	string(aChar)
{
}

QtNormalTextPainter::QtNormalTextPainter(QChar aFirstChar, QChar aNextChar) :
	string(aFirstChar)
{
	string=string.append(aNextChar);
}

void QtNormalTextPainter::paint(const QPoint& aPoint, QPainter& aPainter, const QFont& aFontLower)
{
	Q_UNUSED(aFontLower)

	aPainter.drawText(QPoint(aPoint.x(), aPoint.y()+aPainter.fontMetrics().ascent()), string);
}

int QtNormalTextPainter::width(QPainter& aPainter, const QFont& aFontLower)
{
	Q_UNUSED(aFontLower)

	return aPainter.fontMetrics().width(string);
}
