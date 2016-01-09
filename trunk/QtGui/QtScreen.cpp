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

extern "C"
{
#include <stdint.h>
}

#include "QtScreen.h"
#include "QtEmulator.h"
#include "QtEmulatorAdapter.h"
#include "QtTextPainter.h"
#include "QtScreen.h"

extern "C"
{
	uint64_t LcdData[10];
}

#define SCREEN_ROW_COUNT 10
#define SCREEN_COLUMN_COUNT 40

#define FONT_FILENAME "DejaVuSans.ttf"
#define DEFAULT_FONT_FAMILY "Helvetica"
#define FONT_STYLE QFont::SansSerif

#define NUMBER_FONT_FILENAME "luxisr.ttf"
#define DEFAULT_NUMBER_FONT_FAMILY "Courier"
#define NUMBER_FONT_STYLE QFont::Monospace

static int NON_PIXEL_INDEXES[] = {
	39, 79, 118, 119, 159, 199, 239, 279, 319, 359, 399
};

static int SPECIAL_DIGIT_INDEXES[] = { 1, 2, 3, 4, 5, 6, 7, 8, 24, 25 };


QtScreen::QtScreen(const QtSkin& aSkin, bool anUseFonts)
: useFonts(anUseFonts),
  font(NULL),
  fontLower(NULL),
  smallFont(NULL),
  smallFontLower(NULL),
  numberFont(NULL),
  exponentFont(NULL),
  menuFont(NULL),
  menuFontLower(NULL),
  menuMargin(0),
  menuWidth(0),
  specialDigitPainter(NULL)
{
	setSkin(aSkin);
	for(int i=0; i<(int) (sizeof(NON_PIXEL_INDEXES)/sizeof(int)); i++)
	{
		nonPixelIndexes << NON_PIXEL_INDEXES[i];
	}
	for(int i=0; i<(int) (sizeof(SPECIAL_DIGIT_INDEXES)/sizeof(int)); i++)
	{
		specialDigitIndexes << SPECIAL_DIGIT_INDEXES[i];
	}
}

bool QtScreen::isUseFonts() const
{
	return useFonts;
}

void QtScreen::setUseFonts(bool anUseFonts)
{
	if(useFonts!=anUseFonts)
	{
		useFonts=anUseFonts;
		updateScreen();
	}
}

void QtScreen::setSkin(const QtSkin& aSkin)
{
	screenRectangle=aSkin.getScreenRectangle();
	screenForeground=aSkin.getSCreenForeground();
	screenBackground=aSkin.getSCreenBackground();
	dotPainters=aSkin.getDotPainters();
	pasteRectangle=aSkin.getPasteRectangle();
	pastePainters=aSkin.getPastePainters();

	textOrigin=aSkin.getTextPosition();

	QString fontFamily=DEFAULT_FONT_FAMILY;
	QFont::Weight smallWeight=QFont::Normal;
	QFile fontFile(QString(FONT_FILE_TYPE)+':'+FONT_FILENAME);
	if(fontFile.exists() && fontFile.open(QFile::ReadOnly)) {
		QByteArray data=fontFile.readAll();
		int loadedFontId=QFontDatabase::addApplicationFontFromData(data);
		QStringList loadedFontFamilies=QFontDatabase::applicationFontFamilies(loadedFontId);
		if(!loadedFontFamilies.empty()) {
			fontFamily = loadedFontFamilies.at(0);
			smallWeight=QFont::Bold;
		}
	}

	delete font;
	font=new QFont(fontFamily);
	font->setPixelSize(aSkin.getFontSize());
	font->setStretch(aSkin.getFontStretch());

	delete fontLower;
	fontLower=new QFont(fontFamily);
	fontLower->setPixelSize(aSkin.getFontLowerSize());
	fontLower->setWeight(QFont::Bold);
	fontLower->setStretch(aSkin.getFontStretch());

	delete smallFont;
	smallFont=new QFont(fontFamily);
	smallFont->setPixelSize(aSkin.getSmallFontSize());
	smallFont->setWeight(smallWeight);
	smallFont->setStretch(aSkin.getSmallFontStretch());

	delete smallFontLower;
	smallFontLower=new QFont(fontFamily);
	smallFontLower->setPixelSize(aSkin.getSmallFontLowerSize());
	smallFontLower->setWeight(QFont::Bold);
	smallFont->setStretch(aSkin.getSmallFontStretch());

	numberOrigin=aSkin.getNumberPosition();

	QString numberFontFamily=DEFAULT_NUMBER_FONT_FAMILY;
	QFile numberFontFile(QString(FONT_FILE_TYPE)+':'+NUMBER_FONT_FILENAME);
	if(numberFontFile.exists() && numberFontFile.open(QFile::ReadOnly)) {
		QByteArray data=numberFontFile.readAll();
		int loadedFontId=QFontDatabase::addApplicationFontFromData(data);
		QStringList loadedFontFamilies=QFontDatabase::applicationFontFamilies(loadedFontId);
		if(!loadedFontFamilies.empty()) {
			numberFontFamily = loadedFontFamilies.at(0);
		}
	}

	delete numberFont;
	numberFont=new QFont(numberFontFamily);
	numberFont->setPixelSize(aSkin.getNumberFontSize());
	numberFont->setStretch(aSkin.getNumberFontStretch());
	numberExtraWidth=aSkin.getNumberExtraWidth();
	separatorShift=aSkin.getSeparatorShift();

	exponentOrigin=aSkin.getExponentPosition();
	delete exponentFont;
	exponentFont=new QFont(numberFontFamily);
	exponentFont->setPixelSize(aSkin.getExponentFontSize());
	exponentFont->setStretch(aSkin.getExponentFontStretch());

	delete menuFont;
	menuFont=new QFont(fontFamily);
	menuFont->setPixelSize(aSkin.getMenuFontSize());

	delete menuFontLower;
	menuFontLower=new QFont(fontFamily);
	menuFontLower->setPixelSize(aSkin.getMenuFontLowerSize());

	menuMargin=aSkin.getMenuMargin();
	menuWidth=aSkin.getMenuWidth();

	delete specialDigitPainter;
	specialDigitPainter = new QtSpecialDigitPainter(*numberFont);
}

QtScreen::~QtScreen()
{
	for(DotPainterListConstIterator dotPainterIterator=dotPainters.begin(); dotPainterIterator!=dotPainters.end(); ++dotPainterIterator)
	{
		if(*dotPainterIterator!=NULL)
		{
			delete *dotPainterIterator;
		}
	}
}

const QRect& QtScreen::getScreenRectangle() const
{
	return screenRectangle;
}

QFont& QtScreen::getCatalogMenuFont() const
{
	return *menuFont;
}

QFont& QtScreen::getCatalogMenuFontLower() const
{
	return *menuFontLower;
}

int QtScreen::getCatalogMenuMargin() const
{
	return menuMargin;
}

int QtScreen::getCatalogMenuWidth() const
{
	return menuWidth;
}

void QtScreen::paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent)
{
	Q_UNUSED(aPaintEvent);

	QPainter painter(&aBackgroundImage);
	painter.fillRect(screenRectangle, screenBackground);
	painter.setPen(screenForeground);
	painter.setBrush(QBrush(screenForeground));
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(screenRectangle.topLeft());

	for(int row=0; row<SCREEN_ROW_COUNT; row++)
	{
		for(int column=0; column<SCREEN_COLUMN_COUNT; column++)
		{
	      if((LcdData[row] & ((uint64_t) 1) << column)!=0)
	      {
	    	  int dotIndex=row*SCREEN_COLUMN_COUNT+column;
	    	  if(!shouldUseFonts() || nonPixelIndexes.contains(dotIndex))
	    	  {
	    		  dotPainters[dotIndex]->paint(aBackgroundImage.getBackgroundPixmap(), painter);
	    	  }
	      }
		}
	}
	if(shouldUseFonts())
	{
		char *displayedText = get_last_displayed();
		QFont* currentFontLower;
		if(is_small_font(displayedText))
		{
			painter.setFont(*smallFont);
			currentFontLower=smallFontLower;
		}
		else
		{
			painter.setFont(*font);
			currentFontLower=fontLower;
		}
		int x=textOrigin.x();
		int y=textOrigin.y();
		while(*displayedText!=0)
		{
			char c=*displayedText;
			QtTextPainter* textPainter=QtTextPainter::getTextPainter(c);
			textPainter->paint(QPoint(x, y), painter, *currentFontLower);
			x+=textPainter->width(painter, *currentFontLower);
			displayedText++;
		}

		char *displayedNumber = get_last_displayed_number();
		painter.setFont(*numberFont);
		x=numberOrigin.x();
		y=numberOrigin.y()+painter.fontMetrics().ascent();
		int width=painter.fontMetrics().width('8');
		// First we draw the first character, usually '-' or nothing
		painter.drawText(QPoint(x, y), QString(QChar(*displayedNumber)));
		x+=width;
		displayedNumber++;

		while(*displayedNumber!=0)
		{
			char c=convertCharInDisplayedNumber(*displayedNumber);
			if(specialDigitIndexes.contains(c)) {
				specialDigitPainter->paint(painter, QPoint(x, numberOrigin.y()), getdig(c));
			}
			else
			{
				painter.drawText(QPoint(x+(width-painter.fontMetrics().width(c))/2, y), QString(QChar(c)));
			}
			x+=width;
			displayedNumber++;
			painter.drawText(QPoint(x-separatorShift, y), QString(QChar(*displayedNumber)));
			x+=numberExtraWidth;
			displayedNumber++;
		}

		char *displayedExponent = get_last_displayed_exponent();
		painter.setFont(*exponentFont);
		x=exponentOrigin.x();
		y=exponentOrigin.y()+painter.fontMetrics().ascent();
		width=painter.fontMetrics().width('8');

		while(*displayedExponent!=0)
		{
			painter.drawText(QPoint(x, y), QString(QChar(*displayedExponent)));
			x+=painter.fontMetrics().width(*displayedExponent);
			displayedExponent++;
		}
	}
}

char QtScreen::convertCharInDisplayedNumber(char c) const
{
	// Used to display a degree symbol in HMS mode
	if(c=='@')
	{
		return 0xB0;
	}
	else
	{
		return c;
	}
}

void QtScreen::copy(QtBackgroundImage& aBackgroundImage, QClipboard& aClipboard) const
{
	const QRect* rectangle;
	const DotPainterList* painters;
	if(pasteRectangle.isValid() && !pastePainters.isEmpty())
	{
		rectangle=&pasteRectangle;
		painters=&pastePainters;
	}
	else
	{
		rectangle=&screenRectangle;
		painters=&dotPainters;
	}

	QPixmap pixmap(rectangle->width(), rectangle->height());
	QPainter painter(&pixmap);
	painter.fillRect(pixmap.rect(), screenBackground);
	painter.setPen(screenForeground);
	painter.setBrush(QBrush(screenForeground));
	painter.setRenderHint(QPainter::Antialiasing);

	for(int row=0; row<SCREEN_ROW_COUNT; row++)
	{
		for(int column=0; column<SCREEN_COLUMN_COUNT; column++)
		{
	      if((LcdData[row] & ((uint64_t) 1) << column)!=0)
	      {
	    	  int dotIndex=row*SCREEN_COLUMN_COUNT+column;
	    	  (*painters)[dotIndex]->paint(aBackgroundImage.getBackgroundPixmap(), painter);
	      }
		}
	}
	aClipboard.setPixmap(pixmap);
}

bool QtScreen::shouldUseFonts() const
{
	return useFonts && !isForcedDispPlot();
}

extern "C"
{
	void updateScreen()
	{
		currentEmulator->updateScreen();
	}

	void showMessage(const char* title, const char* message)
	{
		currentEmulator->showMessage(title, message);
	}
}
