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

#ifndef PRINTEREMULATOR_H_
#define PRINTEREMULATOR_H_

#include <QtGui>
#include <QtNetwork>
#include "ScrollablePaper.h"
#include "font82240b.h"

#define PAPER_WIDTH 166
#define LINE_HEIGHT (HP82240B_CHARACTER_HEIGHT+1)
#define PAPER_INITIAL_LINES 20
#define PAPER_HORIZONTAL_MARGIN 10
#define PAPER_VERTICAL_MARGIN 10
#define MAX_LINES 500

#define UDPPORT 5025
#define PRINTER_EMULATOR_TITLE "HP-82240B"

#define WINDOW_SETTINGS_GROUP "MainWindow"
#define WINDOW_GEOMETRY_SETTING "Geometry"


class PrinterEmulator: public QMainWindow
{
Q_OBJECT

public:
	PrinterEmulator();
	~PrinterEmulator();

protected:
	void buildGui();
	void buildSocket();
	void loadSettings();
	void saveSettings();

private slots:
     void processPendingDatagrams();
     void quit();
     void clear();

private:
    QUdpSocket* udpSocket;
    ScrollablePaper* scrollablePaper;
    QSettings settings;
};

#endif /* PRINTEREMULATOR_H_ */
