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

#ifndef QTREGISTERSMODEL_H_
#define QTREGISTERSMODEL_H_

#include <QAbstractTableModel>
#include <QPair>
#include <QList>

class QtRegistersModel: public QAbstractTableModel
{
	Q_OBJECT

public:
	QtRegistersModel(QObject* aParent=0, bool aDisplayAsStack=false);

public:
    int rowCount(const QModelIndex& aParent) const;
    int columnCount(const QModelIndex& aParent) const;
    QVariant data(const QModelIndex& anIndex, int aRole) const;
    QVariant headerData(int aSection, Qt::Orientation anOrientation, int aRole) const;
    void setPrototypeMode(bool aPrototypeMode);
    void refresh();
    bool isDisplayAsStack();
    void setDisplayAsStack(bool aDisplayAsStack);

protected:
    int rowCount() const;
    int columnCount() const;
    QVariant prototypeData(int aColumn) const;

private:
    QList< QPair<QString, int> > displayedRegisters;
    bool prototypeMode;
    bool displayAsStack;
    int lastRowCount;
};

#endif /* QTREGISTERSMODEL_H_ */
