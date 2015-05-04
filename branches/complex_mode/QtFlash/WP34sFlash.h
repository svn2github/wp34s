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

#ifndef WP34S_FLASH_H
#define WP34S_FLASH_H

#include <QThread>
#include <QFile>
#include <exception>
#include <qextserialport.h>
#include "WP34sFlashConsole.h"


#define DEFAULT_TIMEOUT 1000
#define XMODEM_PACKET_SIZE 133
#define XMODEM_DATA_SIZE 128

#define TRANSFER_PACKET_SIZE 257
#define TRANSFER_DATA_SIZE 256

class SerialException: public std::exception
{
public:
	SerialException(const QString& anErrorMessage);
	~SerialException() throw();
	virtual const char* what() const throw();

public:
	QString errorMessage;
};

class WP34sFlash : public QThread
{
	Q_OBJECT

public:
	WP34sFlash(const QString& aFirmwareFilename, const QString & aPortName, bool aDebug);

public:
	void start(WP34sFlashConsole* aConsole);
	int run(WP34sFlashConsole* aConsole);
	void run();
	bool isValid();
	QString errorMessage();

//Protocol
protected:
	void connect() throw(SerialException);
	void sendInitialInstructions() throw(SerialException);
	void sendReceivingProgram() throw(SerialException);
	void sendFirmware() throw(SerialException);
	void sendFirmwareInit() throw(SerialException);
	QByteArray loadFirmwareFile() throw(SerialException);
	void xsend(QByteArray& aByteArray, quint32 aPosition) throw(SerialException);;
	void xsendInit(QByteArray& aByteArray, quint32 aPosition) throw(SerialException);;
	void xsendEnd()  throw(SerialException);;
	quint16 xmodemCRC(const QByteArray aByteArray, int aStart, int aLength);
	unsigned char firmwareCRC(const QByteArray aByteArray, int aStart, int aLength);

protected:
	void report(const QString& aString);
	void reportError(const QString& aString);
	void reportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error=false);
	void prepareProgressReport(int totalKilobytes);
	void reportProgress(int kilobytes);
	void openPort() throw(SerialException);
	void reopenPort() throw(SerialException);
	void closePort();
	QByteArray read(int aCounter, qint64 aTimeout=DEFAULT_TIMEOUT);
	int write(const QByteArray& aByteArray);
	void flush();

private:
	QFile firmwareFile;
	QString portName;
	QextSerialPort* port;
	WP34sFlashConsole* console;
	bool debug;
	int status;
	QString error;
};

#endif /* WP34S_FLASH_H */
