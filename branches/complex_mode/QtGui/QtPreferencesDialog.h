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
#define TOOLS_TAB_NAME "Tools"
#define USE_FONTS_LABEL_TEXT "Use real fonts"
#define SHOW_CATALOG_MENUS_LABEL_TEXT "Use real menus to display catalogs"
#define CLOSE_CATALOG_MENUS_LABEL_TEXT "Close catalog after menu"
#define DISPLAY_AS_STACK_LABEL_TEXT "Display as Stack in Debug (X at bottom)"
#define HSHIFT_DELAY_LABEL_TEXT "H-Shift Delay"
#define USE_H_CLICK_TEXT "Use H-shift click"
#define ALWAYS_USE_H_CLICK_TEXT "Always use H-shift click"
#define USE_TOOLTIPS_LABEL_TEXT "Show Tooltips"
#define SERIAL_PORT_TAB_NAME "Serial Port"
#define SERIAL_PORT_NAME_LABEL_TEXT "Serial Port Name"
#define USE_CUSTOM_DIRECTORY_TEXT "Use a custom directory for memory files"
#define CHOOSE_DIRECTORY_TEXT "Choose"
#define USE_CUSTOM_TOOLS_TEXT "Use a custom directory for tools"

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
			bool anUseHShiftClickFlag,
			bool anAlwaysUseHShiftFlag,
			int anHShiftDelay,
			bool aShowToolTipFlag,
			bool anUseFontsFlag,
			bool aShowCatalogMenusFlag,
			bool aCloseatalogMenusFlag,
			bool aDisplayAsStackFlag,
			const QString& aSerialPortName,
			bool aCustomToolsActiveFlag,
			const QString& aCustomToolsName,
			QWidget* aParent=NULL);
	~QtPreferencesDialog();

public:
	bool isCustomDirectoryActive() const;
	QString getCustomDirectoryName() const;
	bool isUseHShiftClickActive() const;
	bool isAlwaysUseHShiftClickActive() const;
	bool isShowToolTips() const;
	bool isUseFonts() const;
	bool isShowCatalogMenus() const;
	bool isSCloseCatalogMenus() const;
	bool isDisplayAsStack() const;
	int getHShiftDelay() const;
	QString getSerialPortName() const;
	bool isToolsActive() const;
	QString getToolsName() const;

private slots:
	void customDirectoryToggled(bool aButtonChecked);
	void chooseDirectory();
	void serialPortChanged(const QString& aSerialPortName);
	void useHShiftClickToggled(bool aButtonChecked);
	void customToolsToggled(bool aButtonChecked);
	void chooseTools();

private:
	void buildComponents(bool aCustomDirectoryActiveFlag,
			const QString& aCustomDirectoryName,
			bool anUseHShiftClickFlag,
			bool anAlwaysUseHShiftClickFlag,
			bool aShowToolTipFlag,
			bool anUseFontsFlag,
			bool aShowCatalogMenusFlag,
			bool aCloseCatalogMenusFlag,
			bool aDisplayAsStackFlag,
			int anHShiftDelay,
			const QString& aSerialPortName,
			bool aCustomToolsActiveFlag,
			const QString& aCustomToolsName);
	QWidget* buildMemoryTab(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName);
	QWidget* buildKeyboardTab(bool anUseHShiftClickFlag, bool anAlwaysUseHShiftClickFlag, int anHShiftDelay, bool aShowToolTipFlag);
	QWidget* buildDisplayTab(bool anUseFontsFlag, bool aShowCatalogMenusFlag, bool aCloseCatalogMenusFlag, bool aDisplayAsStackFlag);
	QWidget* buildSerialTab(const QString& aSerialPortName);
	QWidget* buildToolsTab(bool aCustomToolsActiveFlag, const QString& aCustomToolsName);
	void fillSerialPorts(QListWidget& aListWidget);

private:
	QRadioButton* useCustomDirectoryButton;
	QLineEdit* directoryNameEdit;
	QPushButton* chooseButton;
	QCheckBox *useHShiftClickButton;
	QCheckBox *alwaysUseHShiftClickButton;
	QCheckBox *showToolTipsClickButton;
	QCheckBox *useFontsClickButton;
	QCheckBox *showCatalogMenusClickButton;
	QCheckBox *closeCatalogMenusClickButton;
	QCheckBox *displayAsStackClickButton;
	QSpinBox* hShiftDelayBox;
	QLineEdit* serialPortNameEdit;
	QRadioButton* useCustomToolsButton;
	QLineEdit* toolsNameEdit;
	QPushButton* chooseToolsButton;
};

#endif /* QTPREFERENCESDIALOG_H_ */
