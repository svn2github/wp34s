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

#include "WP34sTextConsole.h"
#include <QTextStream>
#include <stdio.h>

extern QTextStream qout;
extern QTextStream qerr;

WP34sTextConsole::WP34sTextConsole(bool aProgress)
: progress(aProgress)
{
}

void WP34sTextConsole::report(const QString& aString)
{
	qout << aString << '\n';
	qout.flush();
}

void WP34sTextConsole::reportError(const QString& aString)
{
	qerr << aString << '\n';
	qerr.flush();
}

void WP34sTextConsole::reportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error)
{
	QString string;

	if(!aMessage.isEmpty())
	{
		string+=aMessage+": ";
	}
	for(int i=0; i<aByteArray.length(); i++)
	{
		QChar c(aByteArray[i]);
		unsigned char ascii=c.toAscii();
		string+=QString("0x%1").arg((unsigned int) ascii, 0, 16);
	}
	if(error)
	{
		reportError(string);
	}
	else
	{
		report(string);
	}
}

void WP34sTextConsole::prepareProgressReport(int aTotalKilobytes)
{
	totalKilobytes=aTotalKilobytes;
	progressTimer=new QTime;
	progressTimer->start();
}

void WP34sTextConsole::reportProgress(int kilobytes)
{
	if(progress)
	{
		qint64 elapsed=progressTimer->elapsed()/1000L;
		int percentage=(int) ((100.0*kilobytes)/totalKilobytes);
		QString progressText=QString("%1/%2KB, %3% in %4 sec").arg(kilobytes).arg(totalKilobytes).arg(percentage).arg(elapsed);
		// Should be a safe and very simple way to print the progress text on the same line pending a better solution
		printf("\r%s", progressText.toStdString().c_str());
		fflush(stdout);
	}
}

void WP34sTextConsole::endProgress()
{
	delete progressTimer;
}
