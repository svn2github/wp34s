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

extern "C"
{
	uint64_t LcdData[10];
}

#define SCREEN_ROW_COUNT 10
#define SCREEN_COLUMN_COUNT 40

QtScreen::QtScreen(const QtSkin& aSkin)
{
	setSkin(aSkin);
}

void QtScreen::setSkin(const QtSkin& aSkin)
{
	screenRectangle=aSkin.getScreenRectangle();
	screenForeground=aSkin.getSCreenForeground();
	screenBackground=aSkin.getSCreenBackground();
	dotPainters=aSkin.getDotPainters();
	pasteRectangle=aSkin.getPasteRectangle();
	pastePainters=aSkin.getPastePainters();
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

void QtScreen::paint(QtBackgroundImage& aBackgroundImage, QPaintEvent& aPaintEvent)
{
	Q_UNUSED(aPaintEvent);

	QPainter painter(&aBackgroundImage);
	painter.fillRect(screenRectangle, screenBackground);
	painter.setPen(screenForeground);
	painter.setBrush(QBrush(screenForeground));
	painter.translate(screenRectangle.topLeft());

	for(int row=0; row<SCREEN_ROW_COUNT; row++)
	{
		for(int column=0; column<SCREEN_COLUMN_COUNT; column++)
		{
	      if((LcdData[row] & ((uint64_t) 1) << column)!=0)
	      {
	    	  int dotIndex=row*SCREEN_COLUMN_COUNT+column;
	    	  dotPainters[dotIndex]->paint(aBackgroundImage.getBackgroundPixmap(), painter);
	      }
		}
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

extern "C"
{
	void updateScreen()
	{
		currentEmulator->updateScreen();
	}
}
