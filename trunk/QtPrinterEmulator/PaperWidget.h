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

#ifndef PAPERWIDGET_H_
#define PAPERWIDGET_H_

#include <QtGui>

class PaperWidget : public QWidget
{
public:
	PaperWidget();

public:
    void append(const QByteArray& aByteArray);
    void autoZoom(const QSize& aSize);

protected:
    void buildPixmap(const QSize&);
	void paintEvent(QPaintEvent*);
	void processEscape(int aChar);
	void resetPrinter();
	void selfTest();
	void lineFeed();
	void endOfLine();
	void processNormalChar(int aChar);
	void processGraphics(int aChar);
	void drawPoint(int anX, int anY);
	void drawHorizontalLine(int anX, int anY, int aLength);

private:
	int x, y, xOffset, lineCount, zoom;
    QPixmap* pixmap;
    QPainter* painter;
    QByteArray printedText;
    QByteArray appendedText;
    bool lastIsEscape;
    bool ecma94;
    bool underlined;
    bool expanded;
    int expectedGraphicsChars;
};

#endif /* PAPERWIDGET_H_ */
