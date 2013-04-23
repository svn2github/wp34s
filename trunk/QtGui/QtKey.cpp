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

#include "QtKey.h"

QtKey::QtKey(int aCode, const QRect& aRectangle)
	: code(aCode), rectangle(aRectangle)
{
}

int QtKey::getCode() const
{
	return code;
}

const QRect& QtKey::getRectangle() const
{
	return rectangle;
}

const KeySequenceList& QtKey::getKeySequences() const
{
	return keySequences;
}

void QtKey::addKeySequence(const QKeySequence& aKeySequence)
{
	// As we do not know if aKeySequence has been allocated on the stack or will be destroyed
	// we make a copy of it.
	QKeySequence* keySequence=new QKeySequence(aKeySequence);
	keySequences.append(*keySequence);
}

QString QtKey::getShortcut() const
{
	return shortcut;
}

void QtKey::addShortcut(const QString& aShortcut)
{
	if(shortcut.isEmpty())
	{
		shortcut=aShortcut;
	}
}
