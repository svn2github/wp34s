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

#ifndef QTSERIALPORT_H_
#define QTSERIALPORT_H_

#include <QtCore>
#if HAS_SERIAL
#include <qextserialport.h>
#include "QtSerialPortHelper.h"
#endif

#define WAIT_SERIAL_BUFFER_TIME 10

// We inherit from QThread just to call msleep
class QtSerialPort: public QThread
{
	Q_OBJECT

public:
	QtSerialPort();
	~QtSerialPort();

public:
	const QString& getSerialPortName() const;
	void setSerialPortName(const QString& aSerialPortName);
#if HAS_SERIAL
	bool open(const PortSettings& thePortSettings);
#endif
	void close();
	void flush();
	void writeByte(unsigned char aByte);

signals:
#if HAS_SERIAL
    void openInEventLoop(const PortSettings& thePortSettings);
#endif
    void closeInEventLoop();
    void flushInEventLoop();
    void writeByteInEventLoop(unsigned char aByte);

private slots:
	void readBytes();
#if HAS_SERIAL
	void onOpenInEventLoop(const PortSettings& thePortSettings);
#endif
	void onCloseInEventLoop();
	void onFlushInEventLoop();
    void onWriteByteInEventLoop(unsigned char aByte);

private:
	QString serialPortName;
#if HAS_SERIAL
	QextSerialPort* serialPort;
#endif
};

#endif /* QTSERIALPORT_H_ */
