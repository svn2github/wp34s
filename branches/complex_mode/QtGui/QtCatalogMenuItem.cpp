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

#include "QtCatalogMenuItem.h"
#include "QtEmulatorAdapter.h"

class CatalogLabel: public QLabel
{
public:
	CatalogLabel(QWidget* parent);
protected:
	void paintEvent(QPaintEvent *);
};

CatalogLabel::CatalogLabel(QWidget* parent)
: QLabel("", parent)
{
}

void CatalogLabel::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
    // We do not draw here because the menu's clipping is not in place and we would end drawing on top of scroll arrows
}


QtCatalogMenuItem::QtCatalogMenuItem(QPixmap aPixmap, QPixmap anHighlightedPixmap, int aCatalogPosition, const QString& aText, QObject* parent)
: QWidgetAction(parent), pixmap(aPixmap), highlightedPixmap(anHighlightedPixmap), widget(NULL), highlighted(false), catalogPosition(aCatalogPosition), text(aText)
{
}

QtCatalogMenuItem::~QtCatalogMenuItem() {
}

int QtCatalogMenuItem::getCatalogPosition() const
{
	return catalogPosition;
}

QString QtCatalogMenuItem::getText() const
{
	return text;
}

QWidget* QtCatalogMenuItem::createWidget(QWidget* parent)
{
	widget = new CatalogLabel(parent);
	widget->setPixmap(pixmap);
	// Next line is needed so the QLabel does not get mouse events. If so, QMenu cannot track mouse
	// to highlight current action
	widget->setAttribute(Qt::WA_TransparentForMouseEvents);
	return widget;
}

void QtCatalogMenuItem::deleteWidget(QWidget* parent)
{
	Q_UNUSED(parent)

	delete widget;
	widget=NULL;
}

void QtCatalogMenuItem::setHighlighted(bool anHighlighted)
{
	highlighted=anHighlighted;
	forward_catalog_selection(catalogPosition);
}

void QtCatalogMenuItem::paint(QPainter& aPainter)
{
	aPainter.drawPixmap(widget->pos(), highlighted?highlightedPixmap:pixmap);
}

