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

#include <QTextStream>
#include <stdlib.h>
#include <stdio.h>
#include "WP34sFlash.h"
#include "WP34sTextConsole.h"


QTextStream qout(stdout, QIODevice::WriteOnly);
QTextStream qerr(stderr, QIODevice::WriteOnly);

char* command;

void usageAndExit()
{
	qerr << "Usage: " << command << "[-debug] [-noprogress] <firmware file> <serial port>";
	exit(-1);
}

int main(int argc, char **argv)
{
	bool debug=false;
	bool progress=true;
	QString firmwareFilename;
	QString portName;

	command=argv[0];
	if(argc<3 || argc>5)
	{
		usageAndExit();
	}
	int i=1;
	while(i<argc-2)
	{
		if(strcmp("-debug", argv[i])==0)
		{
			if(debug)
			{
				usageAndExit();
			}
			else
			{
				debug=true;
			}
		}
		if(strcmp("-noprogress", argv[i])==0)
		{
			if(!progress)
			{
				usageAndExit();
			}
			else
			{
				progress=false;
			}
		}
		i++;
	}
	firmwareFilename=argv[i++];
	portName=argv[i];

	WP34sFlash flash(firmwareFilename, portName, debug);
	WP34sTextConsole console(progress);
	return flash.run(&console);
}
