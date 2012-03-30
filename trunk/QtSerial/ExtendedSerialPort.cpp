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

#include "ExtendedSerialPort.h"
#include <qextserialenumerator.h>
#include <fcntl.h>

ExtendedSerialPort::ExtendedSerialPort(QueryMode mode)
: QextSerialPort(mode)
{
}

ExtendedSerialPort::ExtendedSerialPort(const QString & name, QueryMode mode)
: QextSerialPort(name, mode)
{
}

ExtendedSerialPort::ExtendedSerialPort(PortSettings const& settings, QueryMode mode)
: QextSerialPort(settings, mode)
{
}


ExtendedSerialPort::ExtendedSerialPort(const QString & name, PortSettings const& settings, QueryMode mode)
: QextSerialPort(name, settings, mode)
{
}

ExtendedSerialPort::~ExtendedSerialPort()
{
}

void ExtendedSerialPort::flushBuffers()
{
#ifdef Q_OS_UNIX
	QMutexLocker lock(mutex);
	if(isOpen())
	{
		tcdrain(fd);
	}
#else
	flush();
#endif
}

void ExtendedSerialPort::sendBreak()
{
#ifdef Q_OS_UNIX
	QMutexLocker lock(mutex);
	if(isOpen())
	{
		tcsendbreak(fd, 0);
	}
#else

#endif

}

QStringList ExtendedSerialPort::getSerialPorts()
{
	QStringList portNames;
    QList<QextPortInfo> portsInfos = QextSerialEnumerator::getPorts();
    for (int i = 0; i < portsInfos.size(); i++)
    {
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
    	portNames << portsInfos[i].portName;
#else
    	portNames << portsInfos[i].physName;
#endif
    }
    portNames.sort();
    return portNames;
}
