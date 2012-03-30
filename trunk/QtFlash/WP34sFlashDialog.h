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

#ifndef WP34SFLASHDIALOG_H_
#define WP34SFLASHDIALOG_H_

#include <QtGui>
#include "WP34sFlashConsole.h"
#include "WP34sFlash.h"

#define FLASH_DIALOG_TITLE "Flashing WP34s"
#define DEFAULT_CONSOLE_WIDTH 200

#define CONSOLE_WIDTH 60
#define CONSOLE_PROTOTYPE_CHAR 'X'

class WP34sFlashDialog: public QDialog, WP34sFlashConsole
{
	Q_OBJECT

public:
	WP34sFlashDialog(WP34sFlash& aFlash, QWidget* aParent);

public:
	void report(const QString& aString);
	void reportError(const QString& aString);
	void reportBytes(const QString& aString, const QByteArray& aByteArray, bool error=false);
	void prepareProgressReport(int totalKilobytes);
	void reportProgress(int kilobytes);
	void endProgress();

public:
	void accept();
	void reject();
	void setVisible(bool visible);

protected:
	void closeEvent (QCloseEvent* aCloseEvent);
	void setConsoleWidth(QTextEdit* aConsole);

signals:
	void reportInEventLoop(const QString& aString);
	void reportErrorInEventLoop(const QString& aString);
	void reportBytesInEventLoop(const QString& aString, const QByteArray& aByteArray, bool error);
	void prepareProgressReportInEventLoop(int totalKilobytes);
	void reportProgressInEventLoop(int kilobytes);
	void endProgressInEventLoop();

private slots:
	void onCloseButtonClicked(bool checked);
	void onFlashThreadEnded();
	void onReport(const QString& aMessage);
	void onReportError(const QString& aMessage);
	void onReportBytes(const QString& aMessage, const QByteArray& aByteArray, bool error);
	void onPrepareProgressReport(int totalKilobytes);
	void onReportProgress(int kilobytes);
	void onEndProgress();

private:
	WP34sFlash& flash;
	bool flashStarted;
	bool closeAllowed;
	QProgressBar* progressBar;
	QLabel* progressLabel;
	QTime* progressTimer;
    QTextEdit* console;
    QPushButton* closeButton;
};

#endif /* WP34SFLASHDIALOG_H_ */
