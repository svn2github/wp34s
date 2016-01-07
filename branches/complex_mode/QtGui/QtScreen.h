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
#include "QtSpecialDigitPainter.h"

// We need to forward define it as we are included by QtBackgroundImage.h
class QtBackgroundImage;

class QtScreen
{
public:
	QtScreen(const QtSkin& aSkin, bool anUseFonts);
	~QtScreen();

public:
	const QRect& getScreenRectangle() const;
	void paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent);
	void copy(QtBackgroundImage& aBackgroundImage, QClipboard& aClipboard) const;
	void setSkin(const QtSkin& aSkin);
	bool isUseFonts() const;
	void setUseFonts(bool anUseFonts);
	QFont& getCatalogMenuFont() const;
	QFont& getCatalogMenuFontLower() const;
	int getCatalogMenuMargin() const;
	int getCatalogMenuWidth() const;

private:
	bool shouldUseFonts() const;
	char convertCharInDisplayedNumber(char c) const;

private:
	QRect screenRectangle;
	QRect pasteRectangle;
    QColor screenForeground;
    QColor screenBackground;
    DotPainterList dotPainters;
    DotPainterList pastePainters;
    bool useFonts;
    QSet<int> nonPixelIndexes;
    QSet<int> specialDigitIndexes;
    QPoint textOrigin;
    QFont *font;
    QFont *fontLower;
    QFont *smallFont;
    QFont *smallFontLower;
    QPoint numberOrigin;
    QFont *numberFont;
    int numberExtraWidth;
    int separatorShift;
    QPoint exponentOrigin;
    QFont *exponentFont;
    QFont *menuFont;
    QFont *menuFontLower;
    int menuMargin;
    int menuWidth;
    QtSpecialDigitPainter *specialDigitPainter;
};

extern "C"
{
	extern void updateScreen();
	extern void showMessage(const char* title, const char* message);
}

#endif /* QTSCREEN_H_ */
