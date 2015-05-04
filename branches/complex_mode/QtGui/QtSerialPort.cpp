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
#if HAS_SERIAL
#include <qextserialenumerator.h>
#endif
#include <QList>
#include <QThread>


QtSerialPort::QtSerialPort()
#if HAS_SERIAL
: serialPort(NULL)
#endif
{
#if HAS_SERIAL
	qRegisterMetaType<PortSettings>("PortSettings");
	connect(this, SIGNAL(openInEventLoop(const PortSettings&)), this, SLOT(onOpenInEventLoop(const PortSettings&)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(closeInEventLoop()), this, SLOT(onCloseInEventLoop()), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(flushInEventLoop()), this, SLOT(onFlushInEventLoop()), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(writeByteInEventLoop(unsigned char)), this, SLOT(onWriteByteInEventLoop(unsigned char)), Qt::BlockingQueuedConnection);
#endif
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
#if HAS_SERIAL
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
#endif

#if HAS_SERIAL
void QtSerialPort::onOpenInEventLoop(const PortSettings& thePortSettings)
{
	close();
	serialPort=new QextSerialPort(serialPortName, QextSerialPort::EventDriven);
	serialPort->setBaudRate(thePortSettings.BaudRate);
	serialPort->setDataBits(thePortSettings.DataBits);
	serialPort->setParity(thePortSettings.Parity);
	serialPort->setStopBits(thePortSettings.StopBits);
	serialPort->setFlowControl(FLOW_OFF);
	serialPort->open(QIODevice::ReadWrite);
	if(serialPort->isOpen())
	{
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(readBytes()));
	}
}
#endif

// See comment on open
void QtSerialPort::close()
{
#if HAS_SERIAL
	if(QThread::currentThread() == qApp->thread())
	{
		onCloseInEventLoop();
	}
	else
	{
		emit closeInEventLoop();
	}
#endif
}

void QtSerialPort::onCloseInEventLoop()
{
#if HAS_SERIAL
	if(serialPort!=NULL)
	{
		if(serialPort->isOpen())
		{
			serialPort->close();
		}
		delete serialPort;
		serialPort=NULL;
	}
#endif
}

// See comment on open
void QtSerialPort::flush()
{
#if HAS_SERIAL
	if(QThread::currentThread() == qApp->thread())
	{
		onFlushInEventLoop();
	}
	else
	{
		emit flushInEventLoop();
	}
#endif
}

void QtSerialPort::onFlushInEventLoop()
{
#if HAS_SERIAL
	if(serialPort!=NULL && serialPort->isOpen())
	{
		serialPort->flush();
	}
#endif
}

// See comment on open
void QtSerialPort::writeByte(unsigned char aByte)
{
#if HAS_SERIAL
	if(QThread::currentThread() == qApp->thread())
	{
		onWriteByteInEventLoop(aByte);
	}
	else
	{
		emit writeByteInEventLoop(aByte);
	}
#else
	Q_UNUSED(aByte)
#endif
}

void QtSerialPort::onWriteByteInEventLoop(unsigned char aByte)
{
#if HAS_SERIAL
	char* byteBuffer=(char*) &aByte;
	serialPort->write(byteBuffer, 1);
#else
	Q_UNUSED(aByte)
#endif
}


void QtSerialPort::readBytes()
{
#if HAS_SERIAL
    QByteArray bytes;
    int length= serialPort->bytesAvailable();
    bytes.resize(length);
    serialPort->read(bytes.data(), bytes.size());

    for(int i=0; i<bytes.size(); i++)
    {
    	while(forward_byte_received((unsigned char) bytes[i]))
    	{
    		msleep(WAIT_SERIAL_BUFFER_TIME);
    	}
    }
#endif
}

extern "C"
{

int open_port(int baud, int bits, int parity, int stopbits)
{
#if HAS_SERIAL
	PortSettings portSettings;
	switch(baud)
	{
#ifndef Q_OS_WIN
		case 50: { portSettings.BaudRate=BAUD50; break; }
		case 75: { portSettings.BaudRate=BAUD75; break; }
#endif
		case 110: { portSettings.BaudRate=BAUD110; break; }
#ifndef Q_OS_WIN
		case 134: { portSettings.BaudRate=BAUD134; break; }
		case 150: { portSettings.BaudRate=BAUD150; break; }
		case 200: { portSettings.BaudRate=BAUD200; break; }
#endif
		case 300: { portSettings.BaudRate=BAUD300; break; }
		case 600: { portSettings.BaudRate=BAUD600; break; }
		case 1200: { portSettings.BaudRate=BAUD1200; break; }
#ifndef Q_OS_WIN
		case 1800: { portSettings.BaudRate=BAUD1800; break; }
#endif
		case 2400: { portSettings.BaudRate=BAUD2400; break; }
		case 4800: { portSettings.BaudRate=BAUD4800; break; }
		case 9600: { portSettings.BaudRate=BAUD9600; break; }
#if defined(Q_OS_WIN)
		case 14400: { portSettings.BaudRate=BAUD14400; break; }
#endif
		case 19200: { portSettings.BaudRate=BAUD19200; break; }
		case 38400: { portSettings.BaudRate=BAUD38400; break; }
#if defined(Q_OS_WIN)
		case 56000: { portSettings.BaudRate=BAUD56000; break; }
#endif
		case 57600: { portSettings.BaudRate=BAUD57600; break; }
		case 115200: { portSettings.BaudRate=BAUD115200; break; }
#if defined(Q_OS_WIN)
		case 128000: { portSettings.BaudRate=BAUD128000; break; }
		case 256000: { portSettings.BaudRate=BAUD256000; break; }
#endif
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
#if defined(Q_OS_WIN)
		case 'M': { portSettings.Parity=PAR_MARK; break; }
#endif
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
#else
	Q_UNUSED(baud)
	Q_UNUSED(bits)
	Q_UNUSED(parity)
	Q_UNUSED(stopbits)
	return 1;
#endif
}

void close_port()
{
	currentEmulator->getSerialPort().close();
}

void put_byte(unsigned char byte)
{
	currentEmulator->getSerialPort().writeByte(byte);
}

void flush_comm()
{
	currentEmulator->getSerialPort().flush();
}

static QMutex serialMutex;

void serial_lock()
{
	serialMutex.lock();
}

void serial_unlock()
{
	serialMutex.unlock();
}

void debug_memory(char* string,long pos)
{
  qDebug() << string << "=" << pos;
}

}

