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
	Q_OBJECT

public:
	PaperWidget();

public:
    void append(const QByteArray&);
    void clear();
    QSize sizeHint() const;

signals:
	void printed(int anY);
	void textAppended();

private slots:
	void printAppendedText();

protected:
    int print(QByteArray, int aMaxLines);
    void removeFirstLine();
    void buildPixmap();
    void deletePixmap();
    void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);
	void processEscape(int aChar);
	void resetPrinter();
	void selfTest();
	void lineFeed();
	void endOfLine();
	void processNormalChar(int aChar);
	void processGraphics(int aChar);
	void drawPoint(int anY);
	void drawUnderline();
	int toX(int anX);
	int toY(int anY);


private:
	int x, y, xOffset, lineCount, printedLineCount, zoom;
    QPixmap* pixmap;
    QPainter* painter;
    QByteArray printedText;
    QByteArray appendedText;
    QByteArray textToPrint;
    QMutex textMutex;
    bool lastIsEscape;
    bool ecma94;
    bool underlined;
    int expanded;
    int expectedGraphicsChars;
};

#endif /* PAPERWIDGET_H_ */
