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

#include "QtRegistersModel.h"
#include "QtEmulatorAdapter.h"

QtRegistersModel::QtRegistersModel(QObject* aParent)
: QAbstractTableModel(aParent)
{
	char* registerName=get_register_names();
	int registerIndex=get_first_register_index();
	while(*registerName!=0)
	{
		displayedRegisters.append(QPair<QString, int>(QString(registerName[0]), registerIndex));
		registerName++;
		registerIndex++;
	}
}

int QtRegistersModel::rowCount(const QModelIndex& aParent) const
 {
     Q_UNUSED(aParent);
     return rowCount();
 }

int QtRegistersModel::rowCount() const
 {
     return displayedRegisters.size();
 }

 int QtRegistersModel::columnCount(const QModelIndex& aParent) const
 {
     Q_UNUSED(aParent);
     return columnCount();
 }

 int QtRegistersModel::columnCount() const
 {
	 return 2;
 }

 QVariant QtRegistersModel::data(const QModelIndex& anIndex, int aRole) const
 {
     if (!anIndex.isValid())
     {
         return QVariant();
     }
     if (anIndex.row() >= rowCount() || anIndex.row() < 0)
     {
         return QVariant();
     }
     if (aRole == Qt::DisplayRole)
     {
    	 QPair<QString, int> pair = displayedRegisters.at(anIndex.row());

    	 if (anIndex.column() == 0)
    	 {
			return pair.first;
    	 }
    	 else if (anIndex.column() == 1)
    	 {
			return get_formatted_register(pair.second);
    	 }
     }
     return QVariant();
 }

 QVariant QtRegistersModel::headerData(int aSection, Qt::Orientation anOrientation, int aRole) const
 {
     if (aRole != Qt::DisplayRole)
     {
         return QVariant();
     }

     if (anOrientation == Qt::Horizontal)
     {
         switch (aSection)
         {
             case 0:
             {
                 return tr("Register");
             }
             case 1:
             {
                 return tr("Value");
             }
             default:
             {
                 return QVariant();
             }
         }
     }
     return QVariant();
 }

 void QtRegistersModel::refresh()
 {
 	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
 }


