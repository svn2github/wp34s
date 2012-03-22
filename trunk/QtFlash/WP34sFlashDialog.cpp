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

#include "WP34sFlashDialog.h"

WP34sFlashDialog::WP34sFlashDialog(WP34sFlash& aFlash, QWidget* aParent)
:  QDialog(aParent), flash(aFlash), flashStarted(false), closeAllowed(false)
{
	setWindowTitle(FLASH_DIALOG_TITLE);
	QVBoxLayout* dialogLayout=new QVBoxLayout;
	dialogLayout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(dialogLayout);

	progressBar=new QProgressBar;
	dialogLayout->addWidget(progressBar);
	console=new QTextEdit;
	console->setReadOnly(true);
	console->setLineWrapMode(QTextEdit::FixedColumnWidth);
	console->setLineWrapColumnOrWidth(DEFAULT_CONSOLE_WIDTH);
	dialogLayout->addWidget(console);
	closeButton=new QPushButton("Close");
	closeButton->setEnabled(false);
	connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(onCloseButtonClicked(bool)));

	dialogLayout->addWidget(closeButton);

	connect(this, SIGNAL(reportInEventLoop(const QString&)), this, SLOT(onReport(const QString&)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(reportErrorInEventLoop(const QString&)), this, SLOT(onReportError(const QString&)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(reportBytesInEventLoop(const QString&, const QByteArray&, bool)), this, SLOT(onReportBytes(const QString&, const QByteArray&, bool)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(prepareProgressReportInEventLoop(int)), this, SLOT(onPrepareProgressReport(int)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(reportProgressInEventLoop(int)), this, SLOT(onReportProgress(int)), Qt::BlockingQueuedConnection);
}

void WP34sFlashDialog::accept()
{
	if(closeAllowed)
	{
		QDialog::accept();
	}
}

void WP34sFlashDialog::reject()
{
	if(closeAllowed)
	{
		QDialog::reject();
	}
}

void WP34sFlashDialog::setVisible(bool visible)
{
	QDialog::setVisible(visible);
	if(visible && !flashStarted)
	{
		flashStarted=true;
		connect(&flash, SIGNAL(finished()), this, SLOT(onFlashThreadEnded()));
		connect(&flash, SIGNAL(terminated()), this, SLOT(onFlashThreadEnded()));
		flash.start(this);
	}
}

void WP34sFlashDialog::closeEvent (QCloseEvent* aCloseEvent)
{
	if(closeAllowed)
	{
		QDialog::closeEvent(aCloseEvent);
	}
	else
	{
		aCloseEvent->ignore();
	}
}

void WP34sFlashDialog::onCloseButtonClicked(bool checked)
{
	Q_UNUSED(checked)

	accept();
}

void WP34sFlashDialog::onFlashThreadEnded()
{
	closeAllowed=true;
	closeButton->setEnabled(true);
}

void WP34sFlashDialog::report(const QString& aMessage)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onReport(aMessage);
	}
	else
	{
		emit reportInEventLoop(aMessage);
	}
}

void WP34sFlashDialog::onReport(const QString& aMessage)
{
	console->append(aMessage);
}

void WP34sFlashDialog::reportError(const QString& aMessage)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onReportError(aMessage);
	}
	else
	{
		emit reportErrorInEventLoop(aMessage);
	}
}

void WP34sFlashDialog::onReportError(const QString& aMessage)
{
	QColor previousColor=console->textColor();
	console->setTextColor(Qt::red);
	console->append(aMessage);
	console->setTextColor(previousColor);
}

void WP34sFlashDialog::reportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onReportBytes(aMessage, aByteArray, error);
	}
	else
	{
		emit reportBytesInEventLoop(aMessage, aByteArray, error);
	}
}

void WP34sFlashDialog::onReportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error)
{
	QString string;
	QColor previousColor=console->textColor();

	if(error)
	{
		console->setTextColor(Qt::red);
	}
	if(!aMessage.isEmpty())
	{
		string+=aMessage+": ";
	}
	for(int i=0; i<aByteArray.length(); i++)
	{
		QChar c(aByteArray[i]);
		unsigned char ascii=c.toAscii();
		string+=QString("0x%1").arg((unsigned int) ascii, 0, 16);
	}
	report(string);
	console->setTextColor(previousColor);
}

void WP34sFlashDialog::prepareProgressReport(int totalKilobytes)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onPrepareProgressReport(totalKilobytes);
	}
	else
	{
		emit prepareProgressReportInEventLoop(totalKilobytes);
	}
}

void WP34sFlashDialog::onPrepareProgressReport(int totalKilobytes)
{
	progressBar->setRange(0, totalKilobytes);
	progressBar->setFormat("%v/%mKB, %p%");
}

void WP34sFlashDialog::reportProgress(int kilobytes)
{
	if(QThread::currentThread() == qApp->thread())
	{
		onReportProgress(kilobytes);
	}
	else
	{
		emit reportProgressInEventLoop(kilobytes);
	}
}

void WP34sFlashDialog::onReportProgress(int kilobytes)
{
	progressBar->setValue(kilobytes);
}

