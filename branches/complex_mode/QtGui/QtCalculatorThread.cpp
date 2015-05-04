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

#include "QtCalculatorThread.h"
#include "QtEmulator.h"
#include "QtEmulatorAdapter.h"

extern "C"
{
	extern void add_heartbeat();
}

QtCalculatorThread::QtCalculatorThread(QtEmulator& anEmulator)
: emulator(anEmulator), ended(false)
{
}

void QtCalculatorThread::run()
{
	init_calculator();
	QtKeyboard& keyboard=emulator.getKeyboard();
	while(!isEnded())
	{
		int key=keyboard.waitKey();
		if(key>=0)
		{
			forward_keycode(key);
			emulator.getDebugger().refresh();
		}
	}
}

void QtCalculatorThread::end()
{
	QMutexLocker mutexLocker(&mutex);
	ended=true;
	add_heartbeat();
}

bool QtCalculatorThread::isEnded()
{
	QMutexLocker mutexLocker(&mutex);
	return ended;
}
