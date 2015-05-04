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

#ifndef WP34SFLASHCONSOLE_H_
#define WP34SFLASHCONSOLE_H_

#include <QString>

#define KILOBYTE 1024

class WP34sFlashConsole
{
public:
	virtual ~WP34sFlashConsole();

public:
	virtual void report(const QString& aString)=0;
	virtual void reportError(const QString& aString)=0;
	virtual void reportBytes(const QString& aString, const QByteArray& aByteArray, bool error=false)=0;
	virtual void prepareProgressReport(int totalKilobytes)=0;
	virtual void reportProgress(int kilobytes)=0;
};


#endif /* WP34SFLASHCONSOLE_H_ */
