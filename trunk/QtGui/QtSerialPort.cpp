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

#include <QtCore>
#include "QtSerialPort.h"

QtSerialPort::QtSerialPort()
{
}

extern "C"
{

int open_port( int baud, int bits, int parity, int stopbits )
{
	Q_UNUSED(baud)
	Q_UNUSED(bits)
	Q_UNUSED(parity)
	Q_UNUSED(stopbits)

	return 0;
}

extern void close_port( void )
{
}

void put_byte( unsigned char byte )
{
	Q_UNUSED(byte)
}

void flush_comm( void )
{
}

}
