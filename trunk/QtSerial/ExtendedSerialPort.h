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

#ifndef EXTENDEDSERIALPORT_H_
#define EXTENDEDSERIALPORT_H_

#include "qextserialport.h"
#include <QStringList>

class ExtendedSerialPort: public QextSerialPort
{
public:
	ExtendedSerialPort(QueryMode mode = EventDriven);
	ExtendedSerialPort(const QString & name, QueryMode mode = EventDriven);
	ExtendedSerialPort(PortSettings const& settings, QueryMode mode = EventDriven);
	ExtendedSerialPort(const QString & name, PortSettings const& settings, QueryMode mode = EventDriven);
	~ExtendedSerialPort();

public:
	void flushBuffers();
	static QStringList getSerialPorts();
};

#endif /* EXTENDEDSERIALPORT_H_ */
