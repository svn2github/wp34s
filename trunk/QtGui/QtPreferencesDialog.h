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

#ifndef QTPREFERENCESDIALOG_H_
#define QTPREFERENCESDIALOG_H_

#include <QtGui>

#define PREFERENCES_TITLE "Preferences"
#define MEMORY_TAB_NAME "Memory"
#define KEYBOARD_TAB_NAME "Keyboard"
#define HSHIFT_DELAY_LABEL_TEXT "H-Shift Delay"
#define SERIAL_PORT_TAB_NAME "Serial Port"
#define SERIAL_PORT_NAME_LABEL_TEXT "Serial Port Name"
#define USE_CUSTOM_DIRECTORY_TEXT "Use a custom directory for memory files"
#define CHOOSE_DIRECTORY_TEXT "Choose"

#define HSHIFT_DELAY_MAX 999
#define HSHIFT_DELAY_BOX_LENGTH 3

#define DIRECTORY_NAME_DEFAULT_WIDTH 60
#define DIRECTORY_NAME_DEFAULT_CHAR 'x'

#define HORIZONTAL_SPACING 10

class QtPreferencesDialog: public QDialog
{
	Q_OBJECT

public:
	QtPreferencesDialog(bool aCustomDirectoryActiveFlag,
			const QString& aCustomDirectoryName,
			int anHShiftDelay,
			const QString& aSerialPortName,
			QWidget* aParent=NULL);
	~QtPreferencesDialog();

public:
	bool isCustomDirectoryActive() const;
	QString getCustomDirectoryName() const;
	int getHShiftDelay() const;
	QString getSerialPortName() const;

private slots:
	void customDirectoryToggled(bool aButtonChecked);
	void chooseDirectory();
	void serialPortChanged(const QString& aSerialPortName);

private:
	void buildComponents(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName, int anHShiftDelay, const QString& aSerialPortName);
	QWidget* buildMemoryTab(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName);
	QWidget* buildKeyboardTab(int anHShiftDelay);
	QWidget* buildSerialTab(const QString& aSerialPortName);
	void fillSerialPorts(QListWidget& aListWidget);

private:
	QRadioButton* useCustomDirectoryButton;
	QLineEdit* directoryNameEdit;
	QPushButton* chooseButton;
	QSpinBox* hShiftDelayBox;
	QLineEdit* serialPortNameEdit;
};

#endif /* QTPREFERENCESDIALOG_H_ */
