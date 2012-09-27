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

#include "PrinterEmulator.h"
#include "PrintDataReader.h"

PrinterEmulator::PrinterEmulator()
{
	buildGui();
	buildPrinterDataReader();
}

PrinterEmulator::~PrinterEmulator()
{
	saveSettings();
}

void PrinterEmulator::buildPrinterDataReader()
{
	printDataReader=new PrintDataReader(*this);
	printDataReader->start();
}

void PrinterEmulator::buildGui()
{
	setWindowTitle(PRINTER_EMULATOR_TITLE);

	QWidget* centralWidget=new QWidget;
	QBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(centralWidget);
	setCentralWidget(centralWidget);
	QLayout* centralLayout=new QHBoxLayout;
	centralLayout->setContentsMargins(0, 0, 0, 0);
	centralWidget->setLayout(centralLayout);

	scrollablePaper=new ScrollablePaper;
	centralLayout->addWidget(scrollablePaper);

	QWidget* buttons=new QWidget;
	QVBoxLayout* buttonsLayout=new QVBoxLayout;
	buttons->setLayout(buttonsLayout);
	QPushButton* clearButton=new QPushButton("Clear");
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	QPushButton* quitButton=new QPushButton("Quit");
	connect(quitButton, SIGNAL(clicked()), this, SLOT(quit()));
	buttonsLayout->addWidget(clearButton);
	buttonsLayout->addWidget(quitButton);
	buttonsLayout->setSizeConstraint(QLayout::SetFixedSize);
	centralLayout->addWidget(buttons);

	loadSettings();
}

void PrinterEmulator::loadSettings()
{
    settings.beginGroup(WINDOW_SETTINGS_GROUP);
    QRect geometry=settings.value(WINDOW_GEOMETRY_SETTING, QRect(-1, -1, -1, -1)).toRect();
    if(geometry.width()>=0 && geometry.height()>=0)
    {
    	setGeometry(geometry);
    }
    settings.endGroup();
}

void PrinterEmulator::saveSettings()
{
    settings.beginGroup(WINDOW_SETTINGS_GROUP);
    settings.setValue(WINDOW_GEOMETRY_SETTING, geometry());
    settings.endGroup();
}

void PrinterEmulator::quit()
{
	qApp->quit();
}

void PrinterEmulator::append(const QByteArray& aByteArray)
{
	scrollablePaper->append(aByteArray);
}

void PrinterEmulator::clear()
{
	scrollablePaper->clear();
}
