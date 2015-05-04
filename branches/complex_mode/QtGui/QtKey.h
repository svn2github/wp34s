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

#ifndef QTKEY_H_
#define QTKEY_H_

#include <QRect>
#include <QLinkedList>
#include <QKeySequence>

typedef QLinkedList<QKeySequence> KeySequenceList;
typedef KeySequenceList::const_iterator KeySequenceConstIterator;

class QtKey
{
public:
	QtKey(int aCode, const QRect& aRectangle);

public:
	int getCode() const;
	const QRect& getRectangle() const;
	const KeySequenceList& getKeySequences() const;
	void addKeySequence(const QKeySequence& aKeySequence);
	QString getShortcut() const;
	void addShortcut(const QString& aShortcut);

private:
	int code;
	QRect rectangle;
	KeySequenceList keySequences;
	QString shortcut;
};

#endif /* QTKEY_H_ */
