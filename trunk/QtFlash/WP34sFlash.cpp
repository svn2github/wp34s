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

#include <QElapsedTimer>
#include <QFileInfo>
#include "WP34sFlash.h"

#define HEADER_RETRY_COUNT 3
char header1[] = { 0x80, 0x80 , '#' };
#define HEADER1_TIMEOUT 1
char header2[] = { 'N', '#' };
#define HEADER2_TIMEOUT 10

char headerAck[]= { 10, 13 };
char wAck[]= { '>', 10, 13, 0 };
char xmodemEnd[]= { 4 };
char xmodemPacketAck[]= { 6 };
char transferAck[]= { 'C' };
char transferPacketAck[]= { 'Y' };


char receivingProgramCodes[] = { 0x70, 0x47, 0x78, 0x47,
		0x01, 0x06, 0xA0, 0xE3, 0x02, 0x1C, 0xA0, 0xE3, 0xEB, 0x30, 0xE0, 0xE3, 0x0D, 0x3C, 0xC3, 0xE3, 0x9B,
		0x70, 0xE0, 0xE3, 0x43, 0x20, 0xA0, 0xE3, 0x00, 0x40, 0x93, 0xE5, 0x02, 0x00, 0x14, 0xE3, 0xFC, 0xFF,
		0xFF, 0x0A, 0x08, 0x20, 0x83, 0xE5, 0x00, 0x40, 0xA0, 0xE3, 0x40, 0x20, 0xA0, 0xE3, 0x00, 0x60, 0x93,
		0xE5, 0x01, 0x00, 0x16, 0xE3, 0xFC, 0xFF, 0xFF, 0x0A, 0x04, 0x50, 0x93, 0xE5, 0x05, 0x40, 0x24, 0xE0,
		0x00, 0x60, 0x93, 0xE5, 0x01, 0x00, 0x16, 0xE3, 0xFC, 0xFF, 0xFF, 0x0A, 0x04, 0x60, 0x93, 0xE5, 0x06,
		0x40, 0x24, 0xE0, 0x06, 0x54, 0x85, 0xE0, 0x00, 0x60, 0x93, 0xE5, 0x01, 0x00, 0x16, 0xE3, 0xFC, 0xFF,
		0xFF, 0x0A, 0x04, 0x60, 0x93, 0xE5, 0x06, 0x40, 0x24, 0xE0, 0x06, 0x58, 0x85, 0xE0, 0x00, 0x60, 0x93,
		0xE5, 0x01, 0x00, 0x16, 0xE3, 0xFC, 0xFF, 0xFF, 0x0A, 0x04, 0x60, 0x93, 0xE5, 0x06, 0x40, 0x24, 0xE0,
		0x06, 0x5C, 0x85, 0xE0, 0x04, 0x50, 0x80, 0xE4, 0x01, 0x20, 0x52, 0xE2, 0xE5, 0xFF, 0xFF, 0x1A, 0x00,
		0x20, 0x93, 0xE5, 0x01, 0x00, 0x12, 0xE3, 0xFC, 0xFF, 0xFF, 0x0A, 0x04, 0x20, 0x93, 0xE5, 0x04, 0x00,
		0x52, 0xE1, 0x01, 0x0C, 0x40, 0x12, 0x58, 0x20, 0xA0, 0x13, 0xD7, 0xFF, 0xFF, 0x1A, 0xFF, 0x2C, 0xA0,
		0xE3, 0x0F, 0x28, 0x82, 0xE3, 0x00, 0x20, 0x02, 0xE0, 0x01, 0x2C, 0x42, 0xE2, 0x03, 0x20, 0x82, 0xE2,
		0x5A, 0x24, 0x82, 0xE3, 0x00, 0x20, 0x87, 0xE5, 0x04, 0x20, 0x97, 0xE5, 0x01, 0x00, 0x12, 0xE3, 0xFC,
		0xFF, 0xFF, 0x0A, 0x59, 0x20, 0xA0, 0xE3, 0x01, 0x10, 0x51, 0xE2, 0xCA, 0xFF, 0xFF, 0x1A, 0x5A, 0x04,
		0xA0, 0xE3, 0x01, 0x0C, 0x80, 0xE2, 0x0B, 0x00, 0x80, 0xE2, 0x00, 0x00, 0x87, 0xE5, 0x04, 0x20, 0x97,
		0xE5, 0x01, 0x00, 0x12, 0xE3, 0xFC, 0xFF, 0xFF, 0x0A, 0x04, 0x20, 0x97, 0xE5, 0x01, 0x00, 0x12, 0xE3,
		0xFC, 0xFF, 0xFF, 0x0A, 0xFF, 0x00, 0xE0, 0xE3, 0x02, 0x0C, 0xC0, 0xE3, 0x0D, 0x10, 0xA0, 0xE3, 0xA5,
		0x14, 0x81, 0xE3, 0x00, 0x10, 0x80, 0xE5, 0xFE, 0xFF, 0xFF, 0xEA };


WP34sFlash::WP34sFlash(const QString& aFirmwareFilename, const QString& aPortName, bool aDebug)
: firmwareFile(aFirmwareFilename), portName(aPortName), console(NULL), debug(aDebug)
{
}

bool WP34sFlash::isValid()
{
	if(!firmwareFile.exists())
	{
		error="Firmware file does not exist";
		return false;
	}
	if(!QFileInfo(firmwareFile).isReadable())
	{
		error="Firmware file cannot be read";
		return false;
	}
	error.clear();
	return true;
}

QString WP34sFlash::errorMessage()
{
	return error;
}

void WP34sFlash::start(WP34sFlashConsole* aConsole)
{
	console=aConsole;
	QThread::start();
}

int WP34sFlash::run(WP34sFlashConsole* aConsole)
{
	console=aConsole;
	run();
	return status;
}

void WP34sFlash::run()
{
	try
	{
		openPort();
		connect();
		sendInitialInstructions();
		sendReceivingProgram();
		sendFirmware();
		closePort();
		status=0;
	}
	catch(SerialException& exception)
	{
		status=-1;
		reportError(exception.errorMessage);
		closePort();
	}
}

void WP34sFlash::connect() throw(SerialException)
{
	QByteArray answer;
	report("Trying to connect");
	for(int i=0; i<HEADER_RETRY_COUNT; i++)
	{
		write(QByteArray(header1, sizeof(header1)));
		closePort();
		reopenPort();
		write(QByteArray(header2, sizeof(header2)));
		QThread::msleep(HEADER2_TIMEOUT);

		answer=read(2);
		if(!answer.isEmpty() && answer[0]==headerAck[0] && answer[1]==headerAck[1])
		{
			report("Connected");
			return;
		}
	}
	if(debug)
	{
		if(answer.isEmpty())
		{
			reportError("No answer");
		}
		else
		{
			reportBytes("Invalid answer", answer, true);
		}
	}
	throw *(new SerialException("Cannot connect to WP34s"));
}

void WP34sFlash::sendInitialInstructions() throw(SerialException)
{
	report("\nSending initial instructions");
    write("WFFFFFF60,00000100#"); // Set 1 wait state on flash
    write("WFFFFFF64,5A00010B#"); // Set GPNVM1
    write("WFFFFF124,FFFFFFFF#"); // Disable all IRQ on AIC
    write("WFFFFF080,00000000#"); // Set AIC source vector 0 at 0
    write("WFFFFF128,00000004#"); // AIC_Clear interupt request
    write("wFFFFF84C,4#"); // Get PIOC IRQ request status
    QByteArray answer=read(4);
    if(answer.length()==4)
    {
    	report("Initial instructions sent");
    	return;
    }
    else
    {
    	if(debug)
    	{
    		if(answer.isEmpty())
    		{
    			reportError("No answer");
    		}
    		else
    		{
    			reportBytes("Invalid answer", answer, true);
    		}
    	}
    	throw *(new SerialException("Cannot send initial instructions"));
    }
}

void WP34sFlash::sendReceivingProgram() throw(SerialException)
{
	report("\nSending transfer program");
    write("WFFFFF844,00010000#"); // Disable PIOC IRQ
    QByteArray byteArray(receivingProgramCodes, sizeof(receivingProgramCodes));
    xsend(byteArray, (quint32) 0x200B40);
	report("Transfer program sent and running");
}

void WP34sFlash::sendFirmware() throw(SerialException)
{
	report("\nFlashing firmware");
	sendFirmwareInit();
	QByteArray firmware=loadFirmwareFile();
	prepareProgressReport(firmware.size()/KILOBYTE);

	for(int i=0; i<firmware.size()/TRANSFER_DATA_SIZE; i++)
	{
		QByteArray data=firmware.mid(i*TRANSFER_DATA_SIZE, TRANSFER_DATA_SIZE);
	    write(data);
	    unsigned char crc=firmwareCRC(data, 0, TRANSFER_DATA_SIZE);
	    QByteArray crcArray;
	    crcArray.append((char) crc);
	    write(crcArray);
	    QByteArray answer=read(1);
	    if(answer.isEmpty())
	    {
	    	throw *(new SerialException(QString("No answer to firmware packet %1").arg(i)));
	    }
	    else if(answer[0]!=transferPacketAck[0])
	    {
	    	if(debug)
	     	{
	     		reportBytes(QString("Invalid answer to firmware packet %1").arg(i), answer, true);
	     	}
	     	throw *(new SerialException(QString("Cannot send firmware packet %1").arg(i)));
	    }
	    reportProgress((((float) i)*TRANSFER_DATA_SIZE)/KILOBYTE);
	}
	flushBuffers();
	report("\nFirmware flashed successfully. You can reset your WP34s");
}

void WP34sFlash::sendFirmwareInit() throw(SerialException)
{
    write("G00200B40#"); // execute program
    QByteArray answer=read(1);
    if(answer.isEmpty())
    {
    	throw *(new SerialException("No answer to firmware flashing initialization"));
    }
    else if(answer[0]!=transferAck[0])
    {
    	if(debug)
    	{
    		reportBytes("Invalid answer to firmware flashing initialization", answer, true);
    	}
    	throw *(new SerialException("Cannot initialize firmware flashing"));
    }
}

QByteArray WP34sFlash::loadFirmwareFile() throw(SerialException)
{
	char buffer[TRANSFER_DATA_SIZE];
	int bytesRead;
	QByteArray firmware;

	if(!firmwareFile.open(QIODevice::ReadOnly))
	{
		if(debug)
		{
			reportError(QString("Error whilst opening firmware file: ")+firmwareFile.errorString());
		}
    	throw *(new SerialException("Cannot open firmware file"));
	}
	do
	{
		bytesRead=firmwareFile.read(buffer, TRANSFER_DATA_SIZE);
		if(bytesRead<0)
		{
			if(debug)
			{
				reportError(QString("Error whilst reading firmware file: ")+firmwareFile.errorString());
			}
	    	throw *(new SerialException("Cannot read firmware file"));
		}
		if(bytesRead>0)
		{
			if(bytesRead<TRANSFER_DATA_SIZE)
			{
				memset(buffer+bytesRead, 0, TRANSFER_DATA_SIZE-bytesRead);
				bytesRead=0;
			}
			firmware.append(buffer, TRANSFER_DATA_SIZE);
		}
	}
	while(bytesRead!=0);

	return firmware;
}

void WP34sFlash::xsend(QByteArray& aByteArray, quint32 aPosition) throw(SerialException)
{
	xsendInit(aByteArray, aPosition);

	int packet= 1;
	while((packet-1)*XMODEM_DATA_SIZE<aByteArray.size())
	{
		QByteArray header;
		header+=1;
		header+=packet & 0xff;
		header+=255 - (packet & 0xff);
		write(header);
		QByteArray data=aByteArray.mid((packet-1)*XMODEM_DATA_SIZE, XMODEM_DATA_SIZE);
	    write(data);
	    quint16 crc=xmodemCRC(data, 0, XMODEM_DATA_SIZE);
	    QByteArray crcBytes;
	    crcBytes+=crc>>8;
	    crcBytes+=crc & 0xff;
	    write(crcBytes);
	    QByteArray answer=read(1);
	    if(answer.isEmpty())
	    {
	    	throw *(new SerialException("Timeout whilst sending transfer program"));
	    }
	    else if(answer[0]!=xmodemPacketAck[0])
	    {
	    	if(debug)
	    	{
	    		reportBytes("Invalid answer to xmodem packet for transfer program", answer, true);
	    	}
	    	throw *(new SerialException("Cannot send transfer program"));
	    }
	    else
	    {
	    	packet++;
	    }
	}

	xsendEnd();
}

void WP34sFlash::xsendInit(QByteArray& aByteArray, quint32 aPosition) throw(SerialException)
{
	int initialSize=aByteArray.size();
	aByteArray.resize((initialSize+127) & 0xff80);
	for(int i=initialSize; i<aByteArray.size(); i++)
	{
		aByteArray[i]=0;
	}
	QString sendMessage=QString().sprintf("S%08X,%08X#", aPosition, aByteArray.size());
	write(QByteArray(sendMessage.toAscii()));
	QByteArray answer=read(1);
	if(answer.isEmpty() || answer[0]!=transferAck[0])
	{
		if(debug)
		{
			if(answer.isEmpty())
			{
				reportError("No answer to transfer program initialization");
			}
			else
			{
				reportBytes("Invalid answer to transfer program initialization", answer, true);
			}
		}
		throw *(new SerialException("Cannot send transfer program"));
	}
}

void WP34sFlash::xsendEnd() throw(SerialException)
{
    write(QByteArray(xmodemEnd, sizeof(xmodemEnd)));
    QByteArray answer=read(1);
    if(answer.isEmpty())
    {
    	throw *(new SerialException("Timeout whilst ending transfer program transmission"));
    }
    else if(answer[0]!=xmodemPacketAck[0])
    {
    	if(debug)
    	{
    		reportBytes("Invalid answer to xmodem end of transmission for transfer program", answer, true);
    	}
    	throw *(new SerialException("Cannot end transfer program transmission"));
    }
}


#define CRC16 0x1021

quint16 WP34sFlash::xmodemCRC(const QByteArray aByteArray, int aStart, int aLength)
{
	quint16 crc=0;
	int index=aStart;
	while (aLength>0)
	{
		crc = crc ^ aByteArray[index] << 8;
		aLength--;
		index++;

		for (int i = 0; i < 8; i++)
		{
			if (crc & 0x8000)
			{
				crc = crc << 1 ^ CRC16;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}
	return crc;
}

unsigned char WP34sFlash::firmwareCRC(const QByteArray aByteArray, int aStart, int aLength)
{
		unsigned char crc=0;
		for(int i=aStart; i<aStart+aLength; i++)
		{
			crc^=aByteArray[i];
		}
		return crc;
}


void WP34sFlash::report(const QString& aString)
{
	if(console!=NULL)
	{
		console->report(aString);
	}
}

void WP34sFlash::reportError(const QString& aString)
{
	console->reportError(aString);
}

void WP34sFlash::reportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error)
{
	console->reportBytes(aMessage, aByteArray, error);
}

void WP34sFlash::prepareProgressReport(int totalKilobytes)
{
	console->prepareProgressReport(totalKilobytes);
}

void WP34sFlash::reportProgress(int kilobytes)
{
	console->reportProgress(kilobytes);
}


void WP34sFlash::openPort() throw(SerialException)
{
	port=new ExtendedSerialPort(portName, QextSerialPort::Polling);
    port->setBaudRate(BAUD115200);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
	if(!port->open(QIODevice::ReadWrite))
	{
		throw *(new SerialException("Cannot open serial port"));
	};
}

void WP34sFlash::reopenPort() throw(SerialException)
{
	if(!port->open(QIODevice::ReadWrite))
	{
		throw *(new SerialException("Cannot reopen serial port"));
	}
}

void WP34sFlash::closePort()
{
	port->close();
}

QByteArray WP34sFlash::read(int aCounter, qint64 aTimeout)
{
	QElapsedTimer timer;
	timer.start();
	QByteArray result;
	result.clear();
	while(aCounter>0)
	{
		QByteArray byte=port->read(1);
		if(byte.length()!=0)
		{
			result+=byte;
			aCounter-=byte.length();
		}
		else if(timer.hasExpired(aTimeout))
		{
			break;
		}
	}
	return result;
}

int WP34sFlash::write(const QByteArray& aByteArray)
{
	return port->write(aByteArray);
}

void WP34sFlash::flushBuffers()
{
	port->flushBuffers();
}

SerialException::SerialException(const QString& anErrorMessage)
: errorMessage(anErrorMessage)
{
}

SerialException::~SerialException() throw()
{
}

const char* SerialException::what() const throw()
{
	return errorMessage.toStdString().c_str();
}
