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


#ifndef QTCATALOGMENU_H_
#define QTCATALOGMENU_H_

#include <QtGui>
#include <QLinkedList>
#include <QWidgetAction>
#include <QMenu>
#include "QtCatalogMenuItem.h"

class QtCatalogMenu: public QMenu
{
	Q_OBJECT

public:
	QtCatalogMenu();
	virtual ~QtCatalogMenu();

public:
	void addCatalogMenuItem(QtCatalogMenuItem* menuItem);
	void paintCatalogMenuItems(QPainter& painter);
	void mouseMoveEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);

public slots:
	void onHover(QAction* anAction);

private:
	void checkKeyTimeout();
	void setCatalogPositionByKeys();

private:
	QStyle* proxyStyle;
	QVector<QtCatalogMenuItem*> menuItems;
	QtCatalogMenuItem* lastHighlighted;
	QString keys;
	qint64 lastKeyTime;
};

#endif /* QTCATALOGMENU_H_ */
