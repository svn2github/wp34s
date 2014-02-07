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

#include "PaperWidget.h"
#include "PrinterEmulator.h"
#include "font82240b.h"

#define FIRST_PRINTABLE_CHAR 32
#define ESCAPE_CHAR 27
#define END_OF_LINE 10
#define LINE_FEED 4

#define RESET_PRINTER 255
#define SELF_TEST 254
#define USE_EXPANDED_CHARACTERS 253
#define USE_NORMAL_CHARACTERS 252
#define START_UNDERLINING 251
#define STOP_UNDERLINING 250
#define USE_ECMA94 249
#define USE_ROMAN8 248

#define LINE_WIDTH 166

#define LINE_COUNT_UPDATE 5
#define ALL_LINES 0


// A better implementation would be to have 1 pixmap per line
// in a circular list/vector. And to paint only the need ones in paintEvent

PaperWidget::PaperWidget()
: x(0), y(0), xOffset(0), lineCount(PAPER_INITIAL_LINES), zoom(1), pixmap(0), painter(0), lastIsEscape(false), ecma94(false), underlined(false), expanded(1), expectedGraphicsChars(0)
{
	setMinimumSize(PAPER_WIDTH+PAPER_HORIZONTAL_MARGIN, PAPER_INITIAL_LINES*LINE_HEIGHT+PAPER_VERTICAL_MARGIN);
	connect(this, SIGNAL(textAppended()), this, SLOT(printAppendedText()), Qt::QueuedConnection);
}

QSize PaperWidget::sizeHint() const
{
	return minimumSize();
}

void PaperWidget::append(const QByteArray& aText)
{
	{
		QMutexLocker locker(&textMutex);
		appendedText+=aText;
	}
	emit textAppended();
}

void PaperWidget::printAppendedText()
{
	update();
}

void PaperWidget::clear()
{
	printedText.clear();
	deletePixmap();
	update();
}

// We do not pass a reference as the underlying array can be changed during print to remove the first line
int PaperWidget::print(QByteArray aText, int aMaxLines)
{
	printedLineCount=0;
	int charIndex;
	for (charIndex=0; charIndex < aText.size() && (printedLineCount<aMaxLines || aMaxLines<=0); ++charIndex)
	{
		bool escapeFound=false;
		int c=(unsigned char) aText.at(charIndex);
		if(expectedGraphicsChars>0)
		{
			expectedGraphicsChars--;
			processGraphics(c);
		}
		else if(lastIsEscape)
		{
			processEscape(c);
		}
		else
		{
			switch(c)
			{
			case END_OF_LINE:
			{
				endOfLine();
				break;
			}
			case LINE_FEED:
			{
				lineFeed();
				break;
			}
			case ESCAPE_CHAR:
			{
				escapeFound=true;
				break;
			}
			default:
			{
				processNormalChar(c);
				break;
			}
			}
		}
		lastIsEscape=escapeFound;
	}
	emit printed(toY(y+LINE_HEIGHT));
	return charIndex;
}

void PaperWidget::buildPixmap()
{
	QSize currentSize=size();
	zoom=qMax(1,currentSize.width()/(PAPER_WIDTH+PAPER_HORIZONTAL_MARGIN));
	xOffset=(currentSize.width()-PAPER_HORIZONTAL_MARGIN)%(PAPER_WIDTH*zoom);
	int newHeight=lineCount*LINE_HEIGHT*zoom;
	setMinimumHeight(newHeight);
	deletePixmap();
	// We rebuild the pixmap using the last lineCount before resetting it. This way, initial size should be ok.
	pixmap=new QPixmap(currentSize.width(), qMax(currentSize.height(), newHeight));
	pixmap->fill(Qt::white);
	painter=new QPainter(pixmap);
	painter->setPen(Qt::black);

	x=y=0;
	lineCount=0;

	print(printedText, ALL_LINES);
}

void PaperWidget::deletePixmap()
{
	if(painter!=0)
	{
		delete painter;
		painter=0;
	}
	if(pixmap!=0)
	{
		delete pixmap;
		pixmap=0;
	}
}

void PaperWidget::resizeEvent(QResizeEvent* aResizeEvent)
{
	Q_UNUSED(aResizeEvent)

	deletePixmap();
}

void PaperWidget::paintEvent(QPaintEvent* aPaintEvent)
{
	Q_UNUSED(aPaintEvent)

	if(pixmap==0)
	{
		buildPixmap();
	}


	if(!appendedText.isEmpty())
	{
		QMutexLocker locker(&textMutex);
		textToPrint.append(appendedText);
		appendedText.clear();
	}


	// The idea is to force an update every LINE_COUNT_UPDATE
	// If not, the scroll can be invisible and a whole new block appear on a whole
	// For instance a program listing. OSX seem to be more prone to this.
	// But repainting after every new line is too slow so LINE_COUNT_UPDATE is 5.
	if(!textToPrint.isEmpty())
	{
		int consumed=print(textToPrint, LINE_COUNT_UPDATE);
		printedText.append(textToPrint.data(), consumed);
		textToPrint.remove(0, consumed);
	}

	QPainter paperPainter(this);
	paperPainter.drawPixmap(0, 0, *pixmap);
	if(xOffset>0)
	{
		paperPainter.setPen(Qt::gray);
		paperPainter.drawLine(xOffset, 0, xOffset, height());
	}
	if(!textToPrint.isEmpty())
	{
		update();
	}
}

void PaperWidget::processEscape(int anEscapedChar)
{
	switch(anEscapedChar)
	{
	case RESET_PRINTER:
	{
		resetPrinter();
		break;
	}
	case SELF_TEST:
	{
		selfTest();
		break;
	}
	case USE_EXPANDED_CHARACTERS:
	{
		expanded=2;
		break;
	}
	case USE_NORMAL_CHARACTERS:
	{
		expanded=1;
		break;
	}
	case START_UNDERLINING:
	{
		underlined=true;
		break;
	}
	case STOP_UNDERLINING:
	{
		underlined=false;
		break;
	}
	case USE_ECMA94:
	{
		ecma94=true;
		break;
	}
	case USE_ROMAN8:
	{
		ecma94=false;
		break;
	}
	default:
	{
		if(anEscapedChar<=LINE_WIDTH)
		{
			expectedGraphicsChars=anEscapedChar;
		}
		break;
	}
	}
}

int PaperWidget::toX(int anX)
{
	return PAPER_HORIZONTAL_MARGIN/2+xOffset+anX*zoom;

}

int PaperWidget::toY(int anY)
{
	return PAPER_VERTICAL_MARGIN/2+anY*zoom;
}

void PaperWidget::lineFeed()
{
	if(lineCount>=MAX_LINES)
	{
		removeFirstLine();
	}
	y+=LINE_HEIGHT;
	int newHeight=toY(y+LINE_HEIGHT);
	if(newHeight>=pixmap->height())
	{
		QPixmap* newPixmap=new QPixmap(pixmap->width(), newHeight);
		newPixmap->fill(Qt::white);
		QPainter* newPainter=new QPainter(newPixmap);
		newPainter->setPen(Qt::black);
		newPainter->drawPixmap(0, 0, *pixmap);
		setMinimumHeight(newHeight);
		deletePixmap();
		pixmap=newPixmap;
		painter=newPainter;
	}
	x=0;
	lineCount++;
	printedLineCount++;
}

void PaperWidget::removeFirstLine()
{
	for (int charIndex = 0; charIndex < printedText.size(); ++charIndex)
	{
		int c=printedText.at(charIndex);
		if(c==LINE_FEED || c==END_OF_LINE)
		{
			printedText.remove(0, charIndex+1);
			break;
		}
	}

	QPixmap* newPixmap=new QPixmap(pixmap->size());
	newPixmap->fill(Qt::white);
	QPainter* newPainter=new QPainter(newPixmap);
	newPainter->setPen(Qt::black);
	int zoomedLineHeight=toY(LINE_HEIGHT);
	newPainter->drawPixmap(0, toY(0), *pixmap, 0, zoomedLineHeight, pixmap->width(), pixmap->height()-zoomedLineHeight);
	deletePixmap();
	pixmap=newPixmap;
	painter=newPainter;
	lineCount--;
	y-=LINE_HEIGHT;
}

void PaperWidget::endOfLine()
{
	lineFeed();
}

void PaperWidget::processNormalChar(int aChar)
{
	if(aChar>=FIRST_PRINTABLE_CHAR)
	{
		bool firstChar=(x==0);
		if(!firstChar)
		{
			for(int i=0; i<expanded; ++i)
			{
				drawUnderline();
				++x;
			}
		}
		FONTDEF fontDef=(ecma94?sFontEcma94:sFontRoman8)[aChar-FIRST_PRINTABLE_CHAR];
		for(int charX=0; charX<HP82240B_CHARACTER_WIDTH; charX++)
		{
			unsigned char charColumn=fontDef.byCol[charX];
			for(int i=0; i<expanded; ++i)
			{
				unsigned char mask=0x1;
				for(int charY=0; charY<HP82240B_CHARACTER_HEIGHT; charY++, mask <<=1)
				{
					if((charColumn & mask)!=0)
					{
						drawPoint(charY);
					}
				}
				drawUnderline();
				++x;
			}
		}
	}
}

void PaperWidget::drawPoint(int anY)
{
	if(x>=LINE_WIDTH)
	{
		lineFeed();
	}
	painter->fillRect(toX(x), toY(y+anY), zoom, zoom, Qt::black);
}

void PaperWidget::drawUnderline()
{
	if(underlined)
	{
		painter->fillRect(toX(x), toY(y+HP82240B_CHARACTER_HEIGHT-1), zoom, zoom, Qt::black);
	}
}

void PaperWidget::processGraphics(int aChar)
{
	unsigned char charColumn=(unsigned char) aChar;
	unsigned char mask=0x1;
	for(int charY=0; charY<HP82240B_CHARACTER_HEIGHT; charY++, mask <<=1)
	{
		if((charColumn & mask)!=0)
		{
			drawPoint(charY);
		}
	}
	++x;
}

void PaperWidget::resetPrinter()
{
    lastIsEscape=false;
    ecma94=false;
    underlined=false;
    expanded=1;
    expectedGraphicsChars=0;
}

void PaperWidget::selfTest()
{
}
