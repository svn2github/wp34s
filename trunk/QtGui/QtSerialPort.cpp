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

#include "QtSerialPort.h"
#include <qextserialenumerator.h>
#include <QList>
#include "QtEmulator.h"

QtSerialPort::QtSerialPort()
: serialPort(NULL)
{
}

const QString& QtSerialPort::getSerialPortName() const
{
	return serialPortName;
}

void QtSerialPort::setSerialPortName(const QString& aSerialPortName)
{
	serialPortName=aSerialPortName;
}

bool QtSerialPort::open(const PortSettings& thePortSettings)
{
	close();
	serialPort=new QextSerialPort(thePortSettings, QextSerialPort::EventDriven);
	serialPort->setBaudRate(thePortSettings.BaudRate);
	serialPort->setDataBits(thePortSettings.DataBits);
	serialPort->setParity(thePortSettings.Parity);
	serialPort->setStopBits(thePortSettings.StopBits);
	serialPort->setFlowControl(FLOW_OFF);
	bool opened=serialPort->open(QIODevice::ReadWrite);
	if(opened)
	{
// Add connect here
	}
	return opened;
}

void QtSerialPort::close()
{
	if(serialPort!=NULL)
	{
		if(serialPort->isOpen())
		{
			serialPort->close();
		}
		delete serialPort;
		serialPort=NULL;
	}
}

QStringList QtSerialPort::getSerialPorts()
{
	QStringList portNames;
    QList<QextPortInfo> portsInfos = QextSerialEnumerator::getPorts();
    for (int i = 0; i < portsInfos.size(); i++)
    {
    	portNames << portsInfos[i].portName;
    }
    portNames.sort();
    return portNames;
}

extern "C"
{
int open_port(int baud, int bits, int parity, int stopbits)
{
	PortSettings portSettings;
	switch(baud)
	{
#ifndef Q_WS_WIN
		case 50: { portSettings.BaudRate=BAUD50; break; }
		case 75: { portSettings.BaudRate=BAUD75; break; }
#endif
		case 110: { portSettings.BaudRate=BAUD110; break; }
#ifndef Q_WS_WIN
		case 134: { portSettings.BaudRate=BAUD134; break; }
		case 150: { portSettings.BaudRate=BAUD150; break; }
		case 200: { portSettings.BaudRate=BAUD200; break; }
#endif
		case 300: { portSettings.BaudRate=BAUD300; break; }
		case 600: { portSettings.BaudRate=BAUD600; break; }
		case 1200: { portSettings.BaudRate=BAUD1200; break; }
#ifndef Q_WS_WIN
		case 1800: { portSettings.BaudRate=BAUD1800; break; }
#endif
		case 2400: { portSettings.BaudRate=BAUD2400; break; }
		case 4800: { portSettings.BaudRate=BAUD4800; break; }
		case 9600: { portSettings.BaudRate=BAUD9600; break; }
		case 14400: { portSettings.BaudRate=BAUD14400; break; }
		case 19200: { portSettings.BaudRate=BAUD19200; break; }
		case 38400: { portSettings.BaudRate=BAUD38400; break; }
		case 56000: { portSettings.BaudRate=BAUD56000; break; }
		case 57600: { portSettings.BaudRate=BAUD57600; break; }
#ifndef Q_WS_WIN
		case 76800: { portSettings.BaudRate=BAUD76800; break; }
#endif
		case 115200: { portSettings.BaudRate=BAUD115200; break; }
		case 128000: { portSettings.BaudRate=BAUD128000; break; }
		case 256000: { portSettings.BaudRate=BAUD256000; break; }
		default: { portSettings.BaudRate=BAUD9600; break; }
	}

	switch(bits)
	{
		case 5: { portSettings.DataBits=DATA_5; break; }
		case 6: { portSettings.DataBits=DATA_6; break; }
		case 7: { portSettings.DataBits=DATA_7; break; }
		case 8: { portSettings.DataBits=DATA_8; break; }
		default: { portSettings.DataBits=DATA_8; break; }
	}

	switch(parity)
	{
		case 'E': { portSettings.Parity=PAR_EVEN; break; }
		case 'O': { portSettings.Parity=PAR_ODD; break; }
		case 'M': { portSettings.Parity=PAR_MARK; break; }
		case 'S': { portSettings.Parity=PAR_SPACE; break; }
		case 'N': { portSettings.Parity=PAR_NONE; break; }
		default: { portSettings.Parity=PAR_NONE; break; }
	}

	switch(stopbits)
	{
		case 1: { portSettings.StopBits=STOP_1; break; }
		case 2: { portSettings.StopBits=STOP_2; break; }
		default: { portSettings.StopBits=STOP_1; break; }
	}

	return currentEmulator->getSerialPort().open(portSettings);
}

void close_port()
{
}

void put_byte(unsigned char byte)
{
	Q_UNUSED(byte)
}

void flush_comm(void)
{
}

}
