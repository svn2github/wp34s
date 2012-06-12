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

#include "QtDebugger.h"
#include "QtRegistersModel.h"
#include "QtEmulator.h"


QtDebugger::QtDebugger(QWidget* aParent)
: QTableView(aParent)
{
	setModel(new QtRegistersModel());
	setColumnsSizes();
	setSelectionBehavior(SelectRows);
}

void QtDebugger::refresh()
{
	if(isVisible())
	{
		static_cast<QtRegistersModel*>(model())->refresh();
	}
}

void QtDebugger::setColumnsSizes()
{
	QtRegistersModel* registersModel=static_cast<QtRegistersModel*>(model());
	registersModel->setPrototypeMode(true);
	resizeColumnsToContents();
	registersModel->setPrototypeMode(false);
}

QSize QtDebugger::sizeHint() const
{
	return minimumSizeHint();
}

QSize QtDebugger::minimumSizeHint() const
{
	int width = 0;
	for(int i=0, last=model()->columnCount(); i<last; ++i)
	{
		width += columnWidth(i);
	}

	int height = QTableView::minimumSizeHint().height();

	int doubleFrame = 2 * frameWidth();

	width += verticalHeader()->width() + doubleFrame;
	height += horizontalHeader()->height() + doubleFrame;

	return QSize(width, height);
}
