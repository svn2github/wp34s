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

#include "QtCatalogMenu.h"
#include "QtEmulatorAdapter.h"
#include "QtEmulator.h"

#define KEYS_TIMEOUT 1000 // 1 sec instead of 3 secs for the real thing. Here we are using a real keyboard and 3 secs is too long

// All of this so we can draw the menu real clipping and not over the up & down scroll arrows

class ProxyStyle: public QStyle {
public:
	ProxyStyle(QStyle *aStyle, QtCatalogMenu* aMenu) {
		menu = aMenu;
		style = aStyle;
	}

	void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption* opt,
			QPainter* p, const QWidget* w) const {
		style->drawPrimitive(pe, opt, p, w);
	}

	QRect subElementRect(QStyle::SubElement subElement, const QStyleOption* styleOption, const QWidget* widget) const
	{
		Q_UNUSED(subElement);
		Q_UNUSED(styleOption);
		Q_UNUSED(widget);
		return style->subElementRect(subElement, styleOption, widget);
	}

	void drawControl(ControlElement controlElement, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
	{
		style->drawControl(controlElement, option, painter, widget);

		// This method is called at the end of QMemu::paintEvent and the painter is the correct one especially its clip region
		if(controlElement==QStyle::CE_MenuEmptyArea)
		{
			menu->paintCatalogMenuItems(*painter);
		}
	}

	void drawComplexControl(QStyle::ComplexControl complexControl,
			const QStyleOptionComplex* option,
			QPainter* painter,
			const QWidget* widget) const
	{
		style->drawComplexControl(complexControl, option, painter, widget);
	}

	SubControl hitTestComplexControl(QStyle::ComplexControl complexControl,
			const QStyleOptionComplex* option,
			const QPoint& point,
			const QWidget* widget) const
	{
		return style->hitTestComplexControl(complexControl, option, point, widget);
	}

	QRect subControlRect(QStyle::ComplexControl complexControl,
			const QStyleOptionComplex* option,
			QStyle::SubControl subControl,
			const QWidget* widget) const
	{
		return style->subControlRect(complexControl, option, subControl, widget);
	}

	int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
	{
		return style->pixelMetric(metric, option, widget);
	}

	QSize sizeFromContents(QStyle::ContentsType contentType,
			const QStyleOption* option,
			const QSize& size,
			const QWidget* widget) const
	{
		return style->sizeFromContents(contentType, option, size, widget);
	}

	int styleHint(QStyle::StyleHint hint,
			const QStyleOption* option,
			const QWidget* widget,
			QStyleHintReturn* hintReturn) const
	{
		return style->styleHint(hint, option, widget, hintReturn);
	}

	QPixmap standardPixmap(QStyle::StandardPixmap pixmap, const QStyleOption* option, const QWidget* widget) const
	{
		return style->standardPixmap(pixmap, option, widget);
	}

	QPixmap generatedIconPixmap(QIcon::Mode mode, const QPixmap& pixmap, const QStyleOption* option) const
	{
		return style->generatedIconPixmap(mode, pixmap, option);
	}

private:
	QStyle* style;
	QtCatalogMenu* menu;
};


QtCatalogMenu::QtCatalogMenu()
: proxyStyle(NULL), lastHighlighted(NULL), lastKeyTime(-1)
{
	proxyStyle = new ProxyStyle(style(), this);
    setStyle(proxyStyle);
#ifndef Q_WS_MAC
    // Activate scrollable popup menu
    setStyleSheet("QMenu { menu-scrollable: 1; }");
#endif
    connect(this, SIGNAL(hovered(QAction*)), this, SLOT(onHover(QAction*)));
}

QtCatalogMenu::~QtCatalogMenu()
{
	delete proxyStyle;
}

// This methods cleans the current highlight when the mouse exits the menu
// This is standard behavior but I found no callback to implement it in a better way
void QtCatalogMenu::mouseMoveEvent(QMouseEvent* event)
{
	QMenu::mouseMoveEvent(event);
    QAction *action = actionAt(event->pos());
    if (action==NULL && lastHighlighted!=NULL)
    {
		lastHighlighted->setHighlighted(false);
    }
}

void QtCatalogMenu::keyPressEvent(QKeyEvent* event)
{
	QString text=event->text();
	if(!text.isEmpty() && text.length()==1 && text.at(0).isPrint())
	{
		checkKeyTimeout();
		keys+=text.at(0);
		setCatalogPositionByKeys();
	}
	else if(currentEmulator->getKeyboard().findKeyCode(*event).getCode()==backspace_code())
	{
		checkKeyTimeout();
		if(!keys.isEmpty())
		{
			keys.remove(keys.length()-1, 1);
			setCatalogPositionByKeys();
		}
	}
	else
	{
		QMenu::keyPressEvent(event);
	}
}

void QtCatalogMenu::checkKeyTimeout()
{
	qint64 currentTime=QDateTime::currentMSecsSinceEpoch();
	if(currentTime-lastKeyTime>KEYS_TIMEOUT)
	{
		keys.clear();
	}
	lastKeyTime=currentTime;
}

void QtCatalogMenu::setCatalogPositionByKeys()
{
	int pos=find_pos(keys.toStdString().c_str());
	if(pos>=0 && pos<menuItems.size())
	{
		setActiveAction(menuItems.at(pos));
	}
}


void QtCatalogMenu::onHover(QAction* anAction)
{
	if(lastHighlighted!=NULL)
	{
		lastHighlighted->setHighlighted(false);
	}
	QtCatalogMenuItem* menuItem=dynamic_cast<QtCatalogMenuItem*>(anAction);
	if(menuItem!=NULL)
	{
		menuItem->setHighlighted(true);
		lastHighlighted=menuItem;
	}
}

void QtCatalogMenu::addCatalogMenuItem(QtCatalogMenuItem* menuItem)
{
	menuItems.append(menuItem);
	addAction(menuItem);
}

void QtCatalogMenu::paintCatalogMenuItems(QPainter& painter)
{
	Q_FOREACH(QtCatalogMenuItem* menuItem, menuItems)
	{
		menuItem->paint(painter);
	}
}
