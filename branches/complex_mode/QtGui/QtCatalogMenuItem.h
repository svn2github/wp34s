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

#ifndef QTCATALOGMENUITEM_H_
#define QTCATALOGMENUITEM_H_

#include "QtGui"

class QtCatalogMenuItem: public QWidgetAction
{
public:
	QtCatalogMenuItem(QPixmap aPixmap, QPixmap anHighlightedPixmap, int aCatalogPosition, const QString& aText, QObject* parent=0);
	virtual ~QtCatalogMenuItem();

public:
	void paint(QPainter& aPainter);
	void setHighlighted(bool anHighlighted);
	int getCatalogPosition() const;
	QString getText() const;

protected:
	QWidget* createWidget(QWidget* parent);
	void deleteWidget(QWidget* parent);

private:
	 QPixmap pixmap;
	 QPixmap highlightedPixmap;
	 QLabel* widget;
	 bool highlighted;
	 int catalogPosition;
	 QString text;
};

#endif /* QTCATALOGMENUITEM_H_ */
