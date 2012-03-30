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

#ifndef WP34SFLASHGUI_H_
#define WP34SFLASHGUI_H_

#include <QtGui>

#define SERIAL_HORIZONTAL_SPACING 10
#define SERIAL_PORT_TAB_NAME "Serial Port"
#define SERIAL_PORT_NAME_LABEL_TEXT "Serial Port Name"

#define FIRMWARE_FILENAME_LABEL_TEXT "File"

#define ORGANIZATION_NAME "WP-34s"
#define APPLICATION_NAME "WP34sFlash"
#define WINDOWS_SETTINGS_GROUP "MainWindow"
#define WINDOWS_POSITION_SETTING "Position"
#define WINDOWS_SIZE_SETTING "Size"
#define FLASH_SETTINGS_GROUP "SerialPort"
#define FIRMWARE_FILENAME_SETTING "Firmware"
#define SERIAL_PORT_NAME_SETTING "SerialPortName"
#define DEBUG_SETTING "Debug"

class WP34sFlashGui: public QMainWindow
{
	Q_OBJECT

public:
	WP34sFlashGui();
    ~WP34sFlashGui();

protected:
	QWidget* buildFirmwareFileComponent();
	QWidget* buildSerialPortComponent();
	QWidget* buildDebugCheckBox();
	QWidget* buildButtons();
    void loadSettings();
    void saveSettings();
    void setGuiInitialValues(const QString& aFirmwareFilename, const QString& aSerialPortName, bool aDebug);

private slots:
	void serialPortChanged(const QString& aSerialPortName);
	void onFileDialogButtonClicked(bool checked);
	void onFlashButtonClicked(bool checked);
	void onQuitButtonClicked(bool checked);

private:
	QLineEdit* firmwareFilenameEdit;
	QLineEdit* serialPortNameEdit;
	QListWidget* serialPortList;
	QCheckBox* debugCheckBox;
    QSettings settings;
};

#endif /* WP34SFLASHGUI_H_ */
