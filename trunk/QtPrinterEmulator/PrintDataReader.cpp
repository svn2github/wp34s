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

#include "PrintDataReader.h"


PrintDataReader::PrintDataReader(PrinterEmulator& aPrinterEmulator)
: udpSocket(0), printerEmulator(aPrinterEmulator)
{
}

void PrintDataReader::run()
{
	buildSocket();
	for(;;)
	{
		udpSocket->waitForReadyRead();
		while (udpSocket->hasPendingDatagrams())
		{
			QByteArray datagram;
			datagram.resize(udpSocket->pendingDatagramSize());
			udpSocket->readDatagram(datagram.data(), datagram.size());
			printerEmulator.append(datagram);
		}
	}
}

void PrintDataReader::buildSocket()
{
	udpSocket = new QUdpSocket();
    udpSocket->bind(QHostAddress::LocalHost, UDPPORT, QUdpSocket::ShareAddress);
}
