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

#ifndef QTDEBUGGER_H_
#define QTDEBUGGER_H_

#include <QtGui>
#include <QTableView>

class QtDebugger: public QTableView
{
public:
	QtDebugger(QWidget* aParent=0, bool aDisplayAsStack=false);

public:
	void refresh();
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    bool eventFilter(QObject *object, QEvent *event);
    bool isDisplayAsStack();
    void setDisplayAsStack(bool aDisplayAsStack);

protected:
	void setColumnsSizes();

protected:
};

#endif /* QTDEBUGGER_H_ */
