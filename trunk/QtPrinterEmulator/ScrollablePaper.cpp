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

#include "ScrollablePaper.h"
#include "PrinterEmulator.h"

ScrollablePaper::ScrollablePaper()
{
	QVBoxLayout* layout=new QVBoxLayout;
	setLayout(layout);
	paper=new PaperWidget();
	scrollArea=new PaperScrollArea();
	scrollArea->setWidget(paper);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	layout->addWidget(scrollArea);
	scrollArea->setMinimumSize(PAPER_WIDTH+2, PAPER_INITIAL_LINES*LINE_HEIGHT+2);
}

void ScrollablePaper::append(const QByteArray& aByteArray)
{
	paper->append(aByteArray);
}

void ScrollablePaper::resizeEvent(QResizeEvent* aResizeEvent)
{
	Q_UNUSED(aResizeEvent)

	// setViewportMargins is called here to force the viewport layout
	scrollArea->setViewportMargins(0, 0, 0, 0);
	paper->autoZoom(scrollArea->viewport()->size());
}

void PaperScrollArea::setViewportMargins(int left, int top, int right, int bottom)
{
	QScrollArea::setViewportMargins(left, top, right, bottom);
}

