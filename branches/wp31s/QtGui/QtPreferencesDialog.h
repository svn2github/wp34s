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
#define DISPLAY_TAB_NAME "Display"
#define KEYBOARD_TAB_NAME "Keyboard"
#define USE_FONTS_LABEL_TEXT "Use real fonts"
#define SHOW_CATALOG_MENUS_LABEL_TEXT "Use real menus to display catalogs"
#define CLOSE_CATALOG_MENUS_LABEL_TEXT "Close catalog after menu"
#define DISPLAY_AS_STACK_LABEL_TEXT "Display as Stack in Debug (X at bottom)"
#define FSHIFT_DELAY_LABEL_TEXT "F-Shift Delay"
#define USE_F_CLICK_TEXT "Use F-shift click"
#define ALWAYS_USE_F_CLICK_TEXT "Always use F-shift click"
#define USE_TOOLTIPS_LABEL_TEXT "Show Tooltips"
#define SERIAL_PORT_TAB_NAME "Serial Port"
#define SERIAL_PORT_NAME_LABEL_TEXT "Serial Port Name"
#define USE_CUSTOM_DIRECTORY_TEXT "Use a custom directory for memory files"
#define CHOOSE_DIRECTORY_TEXT "Choose"

#define FSHIFT_DELAY_MAX 999
#define FSHIFT_DELAY_BOX_LENGTH 3

#define DIRECTORY_NAME_DEFAULT_WIDTH 60
#define DIRECTORY_NAME_DEFAULT_CHAR 'x'

#define HORIZONTAL_SPACING 10

class QtPreferencesDialog: public QDialog
{
	Q_OBJECT

public:
	QtPreferencesDialog(bool aCustomDirectoryActiveFlag,
			const QString& aCustomDirectoryName,
			bool anUseFShiftClickFlag,
			bool anAlwaysUseFShiftFlag,
			int anFShiftDelay,
			bool aShowToolTipFlag,
			bool anUseFontsFlag,
			bool aShowCatalogMenusFlag,
			bool aCloseatalogMenusFlag,
			bool aDisplayAsStackFlag,
			const QString& aSerialPortName,
			QWidget* aParent=NULL);
	~QtPreferencesDialog();

public:
	bool isCustomDirectoryActive() const;
	QString getCustomDirectoryName() const;
	bool isUseFShiftClickActive() const;
	bool isAlwaysUseFShiftClickActive() const;
	bool isShowToolTips() const;
	bool isUseFonts() const;
	bool isShowCatalogMenus() const;
	bool isSCloseCatalogMenus() const;
	bool isDisplayAsStack() const;
	int getFShiftDelay() const;
	QString getSerialPortName() const;

private slots:
	void customDirectoryToggled(bool aButtonChecked);
	void chooseDirectory();
	void serialPortChanged(const QString& aSerialPortName);
	void useFShiftClickToggled(bool aButtonChecked);

private:
	void buildComponents(bool aCustomDirectoryActiveFlag,
			const QString& aCustomDirectoryName,
			bool anUseFShiftClickFlag,
			bool anAlwaysUseFShiftClickFlag,
			bool aShowToolTipFlag,
			bool anUseFontsFlag,
			bool aShowCatalogMenusFlag,
			bool aCloseCatalogMenusFlag,
			bool aDisplayAsStackFlag,
			int anFShiftDelay,
			const QString& aSerialPortName);
	QWidget* buildMemoryTab(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName);
	QWidget* buildKeyboardTab(bool anUseFShiftClickFlag, bool anAlwaysUseFShiftClickFlag, int anFShiftDelay, bool aShowToolTipFlag);
	QWidget* buildDisplayTab(bool anUseFontsFlag, bool aShowCatalogMenusFlag, bool aCloseCatalogMenusFlag, bool aDisplayAsStackFlag);
	QWidget* buildSerialTab(const QString& aSerialPortName);
	void fillSerialPorts(QListWidget& aListWidget);

private:
	QRadioButton* useCustomDirectoryButton;
	QLineEdit* directoryNameEdit;
	QPushButton* chooseButton;
	QCheckBox *useFShiftClickButton;
	QCheckBox *alwaysUseFShiftClickButton;
	QCheckBox *showToolTipsClickButton;
	QCheckBox *useFontsClickButton;
	QCheckBox *showCatalogMenusClickButton;
	QCheckBox *closeCatalogMenusClickButton;
	QCheckBox *displayAsStackClickButton;
	QSpinBox* fShiftDelayBox;
	QLineEdit* serialPortNameEdit;
};

#endif /* QTPREFERENCESDIALOG_H_ */
