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
#include <QtCore>

QtRegistersModel::QtRegistersModel(QObject* aParent, bool aDisplayAsStack)
: QAbstractTableModel(aParent), prototypeMode(false), displayAsStack(aDisplayAsStack)
{
	char* registerName=get_register_names();
	int registerIndex=get_first_register_index();
	while(*registerName!=0)
	{
		displayedRegisters.append(QPair<QString, int>(QString(registerName[0]), registerIndex));
		registerName++;
		registerIndex++;
	}
	for(int i=0; i<get_maxnumregs(); i++)
	{
		displayedRegisters.append(QPair<QString, int>(QString("R")+QString("%1").arg(QString::number(i), 2, '0'), i));
	}
	lastRowCount=get_numregs();
}

bool QtRegistersModel::isDisplayAsStack()
{
	return displayAsStack;
}

void QtRegistersModel::setDisplayAsStack(bool aDisplayAsStack)
{
	if(displayAsStack!=aDisplayAsStack)
	{
		displayAsStack=aDisplayAsStack;
		refresh();
	}
}

int QtRegistersModel::rowCount(const QModelIndex& aParent) const
 {
     Q_UNUSED(aParent);
     return rowCount();
 }

int QtRegistersModel::rowCount() const
{
	if(prototypeMode)
	{
		return 1;
	}
	else
	{
		return get_numregs();
	}
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

void QtRegistersModel::setPrototypeMode(bool aPrototypeMode)
{
	prototypeMode=aPrototypeMode;
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
    	int index;
    	if(displayAsStack)
    	{
    		index=rowCount()-1-anIndex.row();
    	}
    	else
    	{
    		index=anIndex.row();
    	}
    	QPair<QString, int> pair = displayedRegisters.at(index);
    	if(prototypeMode)
    	{
    		return prototypeData(anIndex.column());
    	}
    	else if (anIndex.column() == 0)
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

QVariant QtRegistersModel::prototypeData(int aColumn) const
{
	if (aColumn == 0)
	{
		return tr("WWW");
	}
	else if (aColumn == 1)
	{
		return tr("8888888888888888");
	}
	else
	{
		return QVariant();
	}
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
	int currentRowCount=get_numregs();
	if(lastRowCount!=currentRowCount)
	{
		lastRowCount=currentRowCount;
		beginResetModel();
		endResetModel();
	}
	emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}



