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

#ifndef PRINTDATAREADER_H_
#define PRINTDATAREADER_H_

#include <QThread>
#include "PrinterEmulator.h"

#define UDPPORT 5025

class PrinterEmulator;

class PrintDataReader : public QThread
{
public:
	PrintDataReader(PrinterEmulator& aPrinterEmulator);

public:
	void run();

protected:
	void buildSocket();

private:
    QUdpSocket* udpSocket;
    PrinterEmulator& printerEmulator;
};

#endif /* PRINTDATAREADER_H_ */
