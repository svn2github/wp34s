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
#include "qextserialport.h"

class QtSerialPort
{
public:
	QtSerialPort();

public:
	const QString& getSerialPortName() const;
	void setSerialPortName(const QString& aSerialPortName);
	bool open(const PortSettings& thePortSettings);
	void close();

public:
	static QStringList getSerialPorts();

private:
	QString serialPortName;
	QextSerialPort* serialPort;
};

#endif /* QTSERIALPORT_H_ */
