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

#include "QtBackgroundImage.h"
#include "QtEmulator.h"
#include "QtEmulatorAdapter.h"
#include "QtTextPainter.h"

static QPoint MOVE_MARGIN(MOVE_MARGIN_X, MOVE_MARGIN_Y);
static QPoint MOVE_OTHER_MARGIN(MOVE_MARGIN_X, -MOVE_MARGIN_Y);

QtBackgroundImage::QtBackgroundImage(const QtSkin& aSkin,
		QtScreen& aScreen,
		QtKeyboard& aKeyboard,
		bool aShowCatalogMenuFlag,
		bool aCloseCatalogMenuFlag,
		QWidget* aParent)
	: QLabel(aParent), screen(aScreen), keyboard(aKeyboard), dragging(false),
	  catalogPopupLabel(NULL), catalogMenu(NULL), activeMenuItem(NULL),
	  showCatalogMenuFlag(aShowCatalogMenuFlag), closeCatalogMenuFlag(aCloseCatalogMenuFlag)
{
	setSkin(aSkin);
	setPixmap(pixmap);
	setMask(pixmap.mask());
	setFocusPolicy(Qt::StrongFocus);
	setFixedSize(pixmap.size());
	connect(&aKeyboard, SIGNAL(keyPressed()), this, SLOT(updateScreen()));
}

void QtBackgroundImage::setSkin(const QtSkin& aSkin)
{
	if(!pixmap.load(QString(IMAGE_FILE_TYPE)+':'+aSkin.getPictureName()))
	{
		throw *(new QtSkinException(QString("Cannot find picture ")+aSkin.getPictureName()));
	}
	setPixmap(pixmap);
	setMask(pixmap.mask());
	setFixedSize(pixmap.size());
	clearToolTips();
	QtKeyList keys=aSkin.getKeys();
	for(QtKeyConstIterator keyIterator=keys.constBegin(); keyIterator!=keys.constEnd(); ++keyIterator)
	{
		if(*keyIterator!=NULL)
		{
			addToolTip(**keyIterator);
		}
	}
	showToolTips(keyboard.isShowToolTips());
	QRect textRect=QRect(aSkin.getTextPosition(), aSkin.getTextSize());
	delete catalogPopupLabel;
	catalogPopupLabel=new QLabel("", this);
	catalogPopupLabel->resize(textRect.width(), textRect.height());
	catalogPopupLabel->move(textRect.x(), textRect.y());
	catalogPopupLabel->setAutoFillBackground(false);
	catalogPopupLabel->installEventFilter(this);
	QString catalogMenuTooltip="Click";
	if(!aSkin.getCatalogMenuKeys().isEmpty())
	{
		catalogMenuTooltip+=QString(" or press ")+aSkin.getCatalogMenuKeys().first().toString();
	}
	catalogMenuTooltip+=" to display menu in catalog mode";
	catalogPopupLabel->setToolTip(catalogMenuTooltip);
}

bool QtBackgroundImage::isShowCatalogMenu() const
{
	return showCatalogMenuFlag;
}

void QtBackgroundImage::setShowCatalogMenu(bool aShowCatalogMenuFlag)
{
	showCatalogMenuFlag=aShowCatalogMenuFlag;
}

bool QtBackgroundImage::isCloseCatalogMenu() const
{
	return closeCatalogMenuFlag;
}

void QtBackgroundImage::setCloseCatalogMenu(bool aCloseCatalogMenuFlag)
{
	closeCatalogMenuFlag=aCloseCatalogMenuFlag;
}

bool QtBackgroundImage::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == catalogPopupLabel && event->type() == QEvent::MouseButtonPress)
    {
    	showCatalogMenu(true);
        return true;
    }
    return QFrame::eventFilter(obj, event);
}

void QtBackgroundImage::showCatalogMenu(bool force)
{
	if(!showCatalogMenuFlag && !force)
	{
		return;
	}
	else if(is_catalogue_mode())
	{
		keyboard.showCatalogMenu();
		catalogMenu=new QtCatalogMenu;
		int last=current_catalogue_max();
		int maxWidth=0;
		QFont& font=screen.getCatalogMenuFont();
		QFont& fontLower=screen.getCatalogMenuFontLower();
		int width=screen.getCatalogMenuWidth()+2*screen.getCatalogMenuMargin();
		int height=QFontMetrics(font).height();
		QPixmap* pixmaps[last];
		QPixmap* highlightedPixmaps[last];
		QString* menuText[last];
		for(int i=0; i<last; i++)
		{
			const unsigned int op = current_catalogue(i);
			char buf[41];
			catcmd(op, buf);
			menuText[i]=new QString(buf);
	        if(is_complex_mode() && buf[0]!=get_complex_prefix())
	        {
	            memmove(buf+1, buf, 40);
	            buf[0]=get_complex_prefix();
	        }
	        int x=screen.getCatalogMenuMargin();
			pixmaps[i]=new QPixmap(width, height);
			highlightedPixmaps[i]=new QPixmap(width, height);
			// This block is just so the painter is deleted at the end and we can then set the mask
			{
				QPainter painter(pixmaps[i]);
				painter.fillRect(pixmaps[i]->rect(), palette().base());
				QPainter highlighedPainter(highlightedPixmaps[i]);
				highlighedPainter.fillRect(highlightedPixmaps[i]->rect(), palette().highlight());
			}
			QPainter painter(pixmaps[i]);
			QPainter highlighedPainter(highlightedPixmaps[i]);
			painter.setFont(font);
			painter.setBrush(palette().color(QPalette::Text));
			painter.setPen(palette().color(QPalette::Text));
			highlighedPainter.setFont(font);
			highlighedPainter.setBrush(palette().color(QPalette::HighlightedText));
			highlighedPainter.setPen(palette().color(QPalette::HighlightedText));
			for(int j=0, last=strlen(buf); j<last; j++)
			{
				QtTextPainter* textPainter=QtTextPainter::getTextPainter(buf[j]);
				textPainter->paint(QPoint(x, 0), painter, fontLower);
				textPainter->paint(QPoint(x, 0), highlighedPainter, fontLower);
				x+=textPainter->width(painter, fontLower);
			}
			x+=screen.getCatalogMenuMargin();
			maxWidth=qMax(x, maxWidth);
		}
		activeMenuItem=NULL;
		for(int i=0; i<last; i++)
		{
			QtCatalogMenuItem* menuItem=new QtCatalogMenuItem(pixmaps[i]->copy(0, 0, maxWidth, height), highlightedPixmaps[i]->copy(0, 0, maxWidth, height), i, *menuText[i], NULL);
			catalogMenu->addCatalogMenuItem(menuItem);
			if(i==get_catpos())
			{
				activeMenuItem=menuItem;
			}
			delete pixmaps[i];
			delete highlightedPixmaps[i];
			delete menuText[i];
		}
		catalogTriggered = false;
	    connect(catalogMenu, SIGNAL(triggered(QAction*)), this, SLOT(onTrigger(QAction*)));

	    // We set the active menu item 50 ms after we have displayed it. If not, it does not know how to scroll to the active entry
	    QTimer::singleShot(50, this, SLOT(setActiveCatalogMenuItem()));
	    catalogMenu->exec(mapToGlobal(catalogPopupLabel->pos()));
	    disconnect(catalogMenu, SIGNAL(triggered(QAction*)), this, SLOT(onTrigger(QAction*)));
	    delete catalogMenu;
	    catalogMenu=NULL;
	    if(closeCatalogMenuFlag && !catalogTriggered)
	    {
	    	close_catalog();
	    }
	}
	else
	{
		if(catalogMenu!=NULL)
		{
			catalogMenu->close();
			delete catalogMenu;
			catalogMenu=NULL;
		}
	}
}

void QtBackgroundImage::onCatalogStateChanged()
{
	showCatalogMenu(false);
}

void QtBackgroundImage::setActiveCatalogMenuItem()
{
	if(catalogMenu!=NULL && activeMenuItem!=NULL)
	{
		catalogMenu->setActiveAction(activeMenuItem);
	}
}

void QtBackgroundImage::onTrigger(QAction* anAction)
{
	QtCatalogMenuItem* menuItem=dynamic_cast<QtCatalogMenuItem*>(anAction);
	if(menuItem!=NULL)
	{
		execute_catpos(menuItem->getCatalogPosition());
		catalogTriggered = true;
	}
}

void QtBackgroundImage::showToolTips(bool aShowToolTipsFlag)
{
	if(aShowToolTipsFlag)
	{
		for(QList<QLabel*>::iterator labelIterator=tooltipLabels.begin(); labelIterator!=tooltipLabels.end(); ++labelIterator)
		{
			(*labelIterator)->show();
		}
	}
	else
	{
		for(QList<QLabel*>::iterator labelIterator=tooltipLabels.begin(); labelIterator!=tooltipLabels.end(); ++labelIterator)
		{
			(*labelIterator)->hide();
		}
	}
}


void QtBackgroundImage::clearToolTips()
{
	for(QList<QLabel*>::iterator labelIterator=tooltipLabels.begin(); labelIterator!=tooltipLabels.end(); ++labelIterator)
	{
		delete *labelIterator;
	}
	tooltipLabels.clear();
}

void QtBackgroundImage::addToolTip(const QtKey& aKey)
{
	QLabel* label=new QLabel("", this);
	label->resize(aKey.getRectangle().width(), aKey.getRectangle().height());
	label->move(aKey.getRectangle().x(), aKey.getRectangle().y());
	label->setAutoFillBackground(false);
	label->setToolTip(aKey.getShortcut());
	tooltipLabels.append(label);
}

QPixmap& QtBackgroundImage::getBackgroundPixmap()
{
	return pixmap;
}

void QtBackgroundImage::keyPressEvent(QKeyEvent* aKeyEvent)
{
	keyboard.processKeyPressedEvent(*aKeyEvent);
}

void QtBackgroundImage::keyReleaseEvent(QKeyEvent* aKeyEvent)
{
	keyboard.processKeyReleasedEvent(*aKeyEvent);
}

void QtBackgroundImage::mousePressEvent(QMouseEvent* aMouseEvent)
{
	if(!keyboard.processButtonPressedEvent(*aMouseEvent) && aMouseEvent->button()==Qt::LeftButton)
	{
		dragging=true;
		lastDragPosition=aMouseEvent->globalPos();
	}
}

void QtBackgroundImage::mouseReleaseEvent(QMouseEvent* aMouseEvent)
{
	if(!dragging)
	{
		keyboard.processButtonReleasedEvent(*aMouseEvent);
	}
	dragging=false;
}

void QtBackgroundImage::mouseMoveEvent(QMouseEvent* aMouseEvent)
{
	if(!dragging)
	{
		keyboard.processMouseMovedEvent(*aMouseEvent);
	}
	else
	{
		moveWindow(aMouseEvent);
	}
}

void QtBackgroundImage::moveWindow(QMouseEvent* aMouseEvent)
{
	QWidget* parentWindow=window();
	QPoint offset=aMouseEvent->globalPos()-lastDragPosition;
	QPoint topLeft=parentWindow->pos()+parentWindow->rect().topLeft()+MOVE_MARGIN+offset;
	QPoint topRight=parentWindow->pos()+parentWindow->rect().topRight()-MOVE_OTHER_MARGIN+offset;
	QPoint bottomLeft=parentWindow->pos()+parentWindow->rect().bottomLeft()+MOVE_OTHER_MARGIN+offset;
	QPoint bottomRight=parentWindow->pos()+parentWindow->rect().bottomRight()-MOVE_MARGIN+offset;
	QDesktopWidget* desktopWidget=QApplication::desktop();
	if(desktopWidget->availableGeometry(topLeft).contains(topLeft)
			|| desktopWidget->availableGeometry(topRight).contains(topRight)
			|| desktopWidget->availableGeometry(bottomLeft).contains(bottomLeft)
			|| desktopWidget->availableGeometry(bottomRight).contains(bottomRight))
	{
		parentWindow->move(parentWindow->pos()+offset);
	}
	lastDragPosition=aMouseEvent->globalPos();
}

void QtBackgroundImage::mouseDoubleClickEvent(QMouseEvent* aMouseEvent)
{
	keyboard.processDoubleClickEvent(*aMouseEvent);
}

void QtBackgroundImage::paintEvent(QPaintEvent* aPaintEvent)
{
	QLabel::paintEvent(aPaintEvent);
	screen.paint(*this, *aPaintEvent);
	keyboard.paint(*this, *aPaintEvent);
}

void QtBackgroundImage::updateScreen()
{
	update(screen.getScreenRectangle());
}

extern "C"
{

void changed_catalog_state()
{
	currentEmulator->onCatalogStateChanged();
}

}
