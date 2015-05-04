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

#include "QtPreferencesDialog.h"
#include "QtSerialPort.h"

QtPreferencesDialog::QtPreferencesDialog(bool aCustomDirectoryActiveFlag,
		const QString& aCustomDirectoryName,
		bool anUseHShiftClickFlag,
		bool anAlwaysUseHShiftClickFlag,
		int anHShiftDelay,
		bool aShowToolTipFlag,
		bool anUseFontFlag,
		bool aShowCatalogMenusFlag,
		bool aCloseCatalogMenusFlag,
		bool aDisplayAsStackClickFlag,
		const QString& aSerialPortName,
		QWidget* aParent)
: QDialog(aParent)
{
	setWindowTitle(PREFERENCES_TITLE);
	buildComponents(aCustomDirectoryActiveFlag,
			aCustomDirectoryName,
			anUseHShiftClickFlag,
			anAlwaysUseHShiftClickFlag,
			aShowToolTipFlag,
			anUseFontFlag,
			aShowCatalogMenusFlag,
			aCloseCatalogMenusFlag,
			aDisplayAsStackClickFlag,
			anHShiftDelay,
			aSerialPortName);
}

QtPreferencesDialog::~QtPreferencesDialog()
{
}

void QtPreferencesDialog::buildComponents(bool aCustomDirectoryActiveFlag,
		const QString& aCustomDirectoryName,
		bool anUseHShiftClickFlag,
		bool anAlwaysUseHShiftClickFlag,
		bool aShowToolTipFlag,
		bool anUseFontFlag,
		bool aShowCatalogMenusFlag,
		bool aCloseCatalogMenusFlag,
		bool aDisplayAsStackFlag,
		int anHShiftDelay,
		const QString& aSerialPortName)
{
	QVBoxLayout* dialogLayout=new QVBoxLayout;
	dialogLayout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(dialogLayout);


	QTabWidget* tabWidget = new QTabWidget;
	tabWidget->addTab(buildMemoryTab(aCustomDirectoryActiveFlag, aCustomDirectoryName), MEMORY_TAB_NAME);
	tabWidget->addTab(buildKeyboardTab(anUseHShiftClickFlag, anAlwaysUseHShiftClickFlag, anHShiftDelay, aShowToolTipFlag), KEYBOARD_TAB_NAME);
	tabWidget->addTab(buildDisplayTab(anUseFontFlag, aShowCatalogMenusFlag, aCloseCatalogMenusFlag, aDisplayAsStackFlag), DISPLAY_TAB_NAME);
	tabWidget->addTab(buildSerialTab(aSerialPortName), SERIAL_PORT_TAB_NAME);

	dialogLayout->addWidget(tabWidget);

	QDialogButtonBox* buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	dialogLayout->addWidget(buttonBox);
}

QWidget* QtPreferencesDialog::buildMemoryTab(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName)
{
	QWidget* memoryTab=new QWidget;
	QVBoxLayout* memoryTabLayout=new QVBoxLayout;

	useCustomDirectoryButton=new QRadioButton(USE_CUSTOM_DIRECTORY_TEXT);
	connect(useCustomDirectoryButton, SIGNAL(toggled(bool)), this, SLOT(customDirectoryToggled(bool)));
	memoryTabLayout->addWidget(useCustomDirectoryButton);

	QHBoxLayout* directoryLayout=new QHBoxLayout;
	directoryLayout->setSpacing(HORIZONTAL_SPACING);

	directoryNameEdit=new QLineEdit;
	int minimumWidth=directoryNameEdit->fontMetrics().width(DIRECTORY_NAME_DEFAULT_CHAR)*DIRECTORY_NAME_DEFAULT_WIDTH;
	directoryNameEdit->setMinimumWidth(minimumWidth);
	directoryLayout->addWidget(directoryNameEdit);

	chooseButton=new QPushButton(CHOOSE_DIRECTORY_TEXT);
	directoryLayout->addWidget(chooseButton);
	connect(chooseButton, SIGNAL(clicked(bool)), this, SLOT(chooseDirectory()));

	useCustomDirectoryButton->setChecked(aCustomDirectoryActiveFlag);
	directoryNameEdit->setText(aCustomDirectoryName);
	customDirectoryToggled(isCustomDirectoryActive());

	memoryTabLayout->addItem(directoryLayout);
	memoryTab->setLayout(memoryTabLayout);

	return memoryTab;
}

QWidget* QtPreferencesDialog::buildKeyboardTab(bool anUseHShiftClickFlag, bool anAlwaysUseHShiftClickFlag, int anHShiftDelay, bool aShowToolTipFlag)
{
	QWidget* keyboardTab=new QWidget;
	QVBoxLayout* keyboardLayout=new QVBoxLayout;

	useHShiftClickButton=new QCheckBox(USE_H_CLICK_TEXT);
	connect(useHShiftClickButton, SIGNAL(toggled(bool)), this, SLOT(useHShiftClickToggled(bool)));
	keyboardLayout->addWidget(useHShiftClickButton);

	alwaysUseHShiftClickButton=new QCheckBox(ALWAYS_USE_H_CLICK_TEXT);
	keyboardLayout->addWidget(alwaysUseHShiftClickButton);

	QHBoxLayout* hShiftDelayLayout=new QHBoxLayout;
	hShiftDelayLayout->setSpacing(HORIZONTAL_SPACING);

	QLabel* hShiftDelayLabel=new QLabel(HSHIFT_DELAY_LABEL_TEXT);
	hShiftDelayLayout->addWidget(hShiftDelayLabel);

	hShiftDelayBox=new QSpinBox;
	hShiftDelayBox->setRange(0, HSHIFT_DELAY_MAX);
	hShiftDelayBox->setValue(anHShiftDelay);
	hShiftDelayLayout->addWidget(hShiftDelayBox);

	hShiftDelayLayout->addStretch();
	keyboardLayout->addItem(hShiftDelayLayout);

	useHShiftClickButton->setChecked(anUseHShiftClickFlag);
	alwaysUseHShiftClickButton->setChecked(anAlwaysUseHShiftClickFlag);
	useHShiftClickToggled(isUseHShiftClickActive());

	showToolTipsClickButton=new QCheckBox(USE_TOOLTIPS_LABEL_TEXT);
	showToolTipsClickButton->setChecked(aShowToolTipFlag);
	keyboardLayout->addWidget(showToolTipsClickButton);

	keyboardTab->setLayout(keyboardLayout);
	return keyboardTab;
}

QWidget* QtPreferencesDialog::buildDisplayTab(bool anUseFontsFlag, bool aShowCatalogMenusFlag, bool aCloseCatalogMenusFlag, bool aDisplayAsStackClickFlag)
{
	QWidget* displayTab=new QWidget;
	QVBoxLayout* displayLayout=new QVBoxLayout;

	useFontsClickButton=new QCheckBox(USE_FONTS_LABEL_TEXT);
	displayLayout->addWidget(useFontsClickButton);
	useFontsClickButton->setChecked(anUseFontsFlag);

	showCatalogMenusClickButton=new QCheckBox(SHOW_CATALOG_MENUS_LABEL_TEXT);
	displayLayout->addWidget(showCatalogMenusClickButton);
	showCatalogMenusClickButton->setChecked(aShowCatalogMenusFlag);

	closeCatalogMenusClickButton=new QCheckBox(CLOSE_CATALOG_MENUS_LABEL_TEXT);
	displayLayout->addWidget(closeCatalogMenusClickButton);
	closeCatalogMenusClickButton->setChecked(aCloseCatalogMenusFlag);

	displayAsStackClickButton=new QCheckBox(DISPLAY_AS_STACK_LABEL_TEXT);
	displayLayout->addWidget(displayAsStackClickButton);
	displayAsStackClickButton->setChecked(aDisplayAsStackClickFlag);

	displayTab->setLayout(displayLayout);
	return displayTab;
}


QWidget* QtPreferencesDialog::buildSerialTab(const QString& aSerialPortName)
{
	QWidget* serialTab=new QWidget;
	QVBoxLayout* serialTabLayout=new QVBoxLayout;

	QHBoxLayout* serialPortNameLayout=new QHBoxLayout;
	serialPortNameLayout->setSpacing(HORIZONTAL_SPACING);
	QLabel* serialPortNameLabel=new QLabel(SERIAL_PORT_NAME_LABEL_TEXT);
	serialPortNameLayout->addWidget(serialPortNameLabel);
	serialPortNameEdit=new QLineEdit;
	serialPortNameLayout->addWidget(serialPortNameEdit);

	serialPortNameEdit->setText(aSerialPortName);
	serialTabLayout->addItem(serialPortNameLayout);

	QListWidget* serialPortList=new QListWidget;
	serialTabLayout->addWidget(serialPortList);
	fillSerialPorts(*serialPortList);
	QList<QListWidgetItem*> items=serialPortList->findItems(aSerialPortName, Qt::MatchExactly);
	if(!items.isEmpty())
	{
		serialPortList->setCurrentItem(items[0]);
	}

	connect(serialPortList, SIGNAL(currentTextChanged(const QString&)), this, SLOT(serialPortChanged(const QString&)));

	serialTab->setLayout(serialTabLayout);

	return serialTab;
}

void QtPreferencesDialog::fillSerialPorts(QListWidget& aListWidget)
{
#if HAS_SERIAL
	aListWidget.addItems(QtSerialPortHelper::getSerialPorts());
#else
	Q_UNUSED(aListWidget)
#endif
}

bool QtPreferencesDialog::isCustomDirectoryActive() const
{
	return useCustomDirectoryButton->isChecked();
}

QString QtPreferencesDialog::getCustomDirectoryName() const
{
	return directoryNameEdit->text();
}

void QtPreferencesDialog::customDirectoryToggled(bool aButtonChecked)
{
	directoryNameEdit->setReadOnly(!aButtonChecked);
	chooseButton->setEnabled(aButtonChecked);
}

void QtPreferencesDialog::chooseDirectory()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::Directory);
	fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
	if (fileDialog.exec())
	{
		QStringList filenames = fileDialog.selectedFiles();
		if(filenames.count()==1)
		{
			directoryNameEdit->setText(filenames[0]);
		}
	}
}

bool QtPreferencesDialog::isUseFonts() const
{
	return useFontsClickButton->isChecked();
}

bool QtPreferencesDialog::isShowCatalogMenus() const
{
	return showCatalogMenusClickButton->isChecked();
}

bool QtPreferencesDialog::isSCloseCatalogMenus() const
{
	return closeCatalogMenusClickButton->isChecked();
}

bool QtPreferencesDialog::isDisplayAsStack() const
{
	return displayAsStackClickButton->isChecked();
}

void QtPreferencesDialog::useHShiftClickToggled(bool aButtonChecked)
{
	alwaysUseHShiftClickButton->setEnabled(aButtonChecked);
}

bool QtPreferencesDialog::isUseHShiftClickActive() const
{
	return useHShiftClickButton->isChecked();
}

bool QtPreferencesDialog::isAlwaysUseHShiftClickActive() const
{
	return alwaysUseHShiftClickButton->isChecked();
}

int QtPreferencesDialog::getHShiftDelay() const
{
	return hShiftDelayBox->value();
}

bool QtPreferencesDialog::isShowToolTips() const
{
	return showToolTipsClickButton->isChecked();
}


QString QtPreferencesDialog::getSerialPortName() const
{
	return serialPortNameEdit->text();
}

void QtPreferencesDialog::serialPortChanged(const QString& aSerialPortName)
{
	serialPortNameEdit->setText(aSerialPortName);
}
