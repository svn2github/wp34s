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
#include "QtEmulator.h"
#include "QtEmulatorAdapter.h"
#include <qextserialenumerator.h>
#include <QList>
#include <QThread>

static QtSerialPort* currentSerialPort;

QtSerialPort::QtSerialPort()
: serialPort(NULL)
{
	qRegisterMetaType<PortSettings>("PortSettings");
	currentSerialPort=this;
	connect(this, SIGNAL(openInEventLoop(const PortSettings&)), this, SLOT(onOpenInEventLoop(const PortSettings&)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(closeInEventLoop()), this, SLOT(onCloseInEventLoop()), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(flushInEventLoop()), this, SLOT(onFlushInEventLoop()), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(writeByteInEventLoop(unsigned char)), this, SLOT(onWriteByteInEventLoop(unsigned char)), Qt::BlockingQueuedConnection);
}

QtSerialPort::~QtSerialPort()
{
	close();
}

const QString& QtSerialPort::getSerialPortName() const
{
	return serialPortName;
}

void QtSerialPort::setSerialPortName(const QString& aSerialPortName)
{
	serialPortName=aSerialPortName;
}

// Qextserialport must run in the same thread as the event loop because it uses a QSocketNotifier at least
// So we use slots/signals to ensure it.
bool QtSerialPort::open(const PortSettings& thePortSettings)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onOpenInEventLoop(thePortSettings);
	}
	else
	{
		emit openInEventLoop(thePortSettings);
	}

	return serialPort->isOpen();
}

void QtSerialPort::onOpenInEventLoop(const PortSettings& thePortSettings)
{

	close();
	serialPort=new QextSerialPort(serialPortName, QextSerialPort::EventDriven);
	qDebug() << "BaudRate " << thePortSettings.BaudRate << ", DataBits " << thePortSettings.DataBits << ", Parity " << thePortSettings.Parity << ", StopBits " << thePortSettings.StopBits;
//	serialPort->setBaudRate(thePortSettings.BaudRate);
//	serialPort->setDataBits(thePortSettings.DataBits);
//	serialPort->setParity(thePortSettings.Parity);
//	serialPort->setStopBits(thePortSettings.StopBits);
//	serialPort->setFlowControl(FLOW_OFF);
	serialPort->setBaudRate(BAUD9600);
	serialPort->setFlowControl(FLOW_OFF);
	serialPort->setParity(PAR_NONE);
	serialPort->setDataBits(DATA_8);
	serialPort->setStopBits(STOP_1);

	serialPort->open(QIODevice::ReadWrite);
	if(serialPort->isOpen())
	{
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(readBytes()));
	}
}

// See comment on open
void QtSerialPort::close()
{
	if(QThread::currentThread() == qApp->thread())
	{
		onCloseInEventLoop();
	}
	else
	{
		emit closeInEventLoop();
	}

}

void QtSerialPort::onCloseInEventLoop()
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

// See comment on open
void QtSerialPort::flush()
{
	if(QThread::currentThread() == qApp->thread())
	{
		onFlushInEventLoop();
	}
	else
	{
		emit flushInEventLoop();
	}

}

void QtSerialPort::onFlushInEventLoop()
{
	if(serialPort!=NULL && serialPort->isOpen())
	{
		serialPort->flush();
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

// See comment on open
void QtSerialPort::writeByte(unsigned char aByte)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onWriteByteInEventLoop(aByte);
	}
	else
	{
		emit writeByteInEventLoop(aByte);
	}

}

void QtSerialPort::onWriteByteInEventLoop(unsigned char aByte)
{
	char* byteBuffer=(char*) &aByte;
	qDebug() << "Writing " << QString("%1").arg((int) byteBuffer[0], 0, 16);
	serialPort->write(byteBuffer, 1);
	qDebug() << "Written " << QString("%1").arg((int) byteBuffer[0], 0, 16);
}

void QtSerialPort::readBytes()
{
    QByteArray bytes;
    int length= serialPort->bytesAvailable();
    qDebug() << "Received " << length;
    bytes.resize(length);
    serialPort->read(bytes.data(), bytes.size());

    {
    	QDebug debug=qDebug();
    	debug << "bytes: ";
    	for(int i=0; i<bytes.size(); i++)
    	{
    		debug << QString("%1").arg((int) bytes[i], 0, 16) << " ";
    	}
    }
    for(int i=0; i<bytes.size(); i++)
    {
    	while(forward_byte_received(bytes[i]))
    	{
    		msleep(WAIT_SERIAL_BUFFER_TIME);
    	}
    }
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

	// Return 0 if everything was ok, 1 if not
	return currentEmulator->getSerialPort().open(portSettings)?0:1;
}

void close_port()
{
	currentSerialPort->close();
}

void put_byte(unsigned char byte)
{
	currentSerialPort->writeByte(byte);
}

void flush_comm()
{
	currentSerialPort->flush();
}

}

