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

#ifndef WP34STEXTCONSOLE_H_
#define WP34STEXTCONSOLE_H_

#include <QTime>
#include "WP34sFlashConsole.h"

class WP34sTextConsole: public WP34sFlashConsole
{
public:
	WP34sTextConsole(bool aProgress);

public:
	void report(const QString& aString);
	void reportError(const QString& aString);
	void reportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error=false);
	void prepareProgressReport(int aTotalKilobytes);
	void reportProgress(int kilobytes);
	void endProgress();

private:
	bool progress;
	QTime* progressTimer;
	int totalKilobytes;
};

#endif /* WP34STEXTCONSOLE_H_ */
