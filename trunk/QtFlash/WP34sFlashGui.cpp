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

#include "WP34sFlashGui.h"
#include "WP34sFlashDialog.h"
#include <qextserialenumerator.h>
#include "ExtendedSerialPort.h"

WP34sFlashGui::WP34sFlashGui()
{
	QVBoxLayout* layout = new QVBoxLayout();
	QWidget* widget=new QWidget;
	layout->addWidget(widget);
	setCentralWidget(widget);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	QVBoxLayout* widgetLayout=new QVBoxLayout;
	widget->setLayout(widgetLayout);
	widgetLayout->addWidget(buildFirmwareFileComponent());
	widgetLayout->addWidget(buildSerialPortComponent());
	widgetLayout->addWidget(buildDebugCheckBox());
	widgetLayout->addWidget(buildButtons());

	loadSettings();
}

WP34sFlashGui::~WP34sFlashGui()
{
	saveSettings();
}

void WP34sFlashGui::loadSettings()
{
	settings.beginGroup(WINDOWS_SETTINGS_GROUP);
	if(settings.contains(WINDOWS_POSITION_SETTING))
	{
		move(settings.value(WINDOWS_POSITION_SETTING).toPoint());
	}
	if(settings.contains(WINDOWS_SIZE_SETTING))
	{
		resize(settings.value(WINDOWS_SIZE_SETTING).toSize());
	}
	settings.endGroup();

    settings.beginGroup(FLASH_SETTINGS_GROUP);
    QString firmwareFilename=settings.value(FIRMWARE_FILENAME_SETTING, QString()).toString();
    QString serialPortName=settings.value(SERIAL_PORT_NAME_SETTING, QString()).toString();
    bool debug=settings.value(DEBUG_SETTING, false).toBool();
    settings.endGroup();

    setGuiInitialValues(firmwareFilename, serialPortName, debug);
}

void WP34sFlashGui::saveSettings()
{
    settings.beginGroup(WINDOWS_SETTINGS_GROUP);
    settings.setValue(WINDOWS_POSITION_SETTING, pos());
    settings.setValue(WINDOWS_SIZE_SETTING, size());
    settings.endGroup();

    settings.beginGroup(FLASH_SETTINGS_GROUP);
    settings.setValue(FIRMWARE_FILENAME_SETTING, firmwareFilenameEdit->text());
    settings.setValue(SERIAL_PORT_NAME_SETTING, serialPortNameEdit->text());
    settings.setValue(DEBUG_SETTING, debugCheckBox->isChecked());
    settings.endGroup();

	settings.sync();
}

void WP34sFlashGui::setGuiInitialValues(const QString& aFirmwareFilename, const QString& aSerialPortName, bool aDebug)
{
	firmwareFilenameEdit->setText(aFirmwareFilename);
	QList<QListWidgetItem*> items=serialPortList->findItems(aSerialPortName, Qt::MatchExactly);
	if(!items.isEmpty())
	{
		serialPortList->setCurrentItem(items[0]);
	}
	debugCheckBox->setChecked(aDebug);
}

QWidget* WP34sFlashGui::buildFirmwareFileComponent()
{
	QWidget* firmwareFileComponent=new QWidget;
	QHBoxLayout* firmwareFileLayout=new QHBoxLayout;
	firmwareFileComponent->setLayout(firmwareFileLayout);
	firmwareFileLayout->setSpacing(SERIAL_HORIZONTAL_SPACING);

	QLabel* firmwareFilenameLabel=new QLabel(FIRMWARE_FILENAME_LABEL_TEXT);
	firmwareFileLayout->addWidget(firmwareFilenameLabel);

	QWidget* filenameComponent=new QWidget;
	QHBoxLayout* filenameLayout=new QHBoxLayout;
	filenameComponent->setLayout(filenameLayout);
	firmwareFilenameEdit=new QLineEdit;
	filenameLayout->addWidget(firmwareFilenameEdit);
	QPushButton* fileDialogButton=new QPushButton("...");
	connect(fileDialogButton, SIGNAL(clicked(bool)), this, SLOT(onFileDialogButtonClicked(bool)));
	filenameLayout->addWidget(fileDialogButton);
	firmwareFileLayout->addWidget(filenameComponent);

	return firmwareFileComponent;
}

QWidget* WP34sFlashGui::buildSerialPortComponent()
{
	QWidget* serialPortChooser=new QWidget;
	QVBoxLayout* serialPortChooserLayout=new QVBoxLayout;
	serialPortChooser->setLayout(serialPortChooserLayout);

	QWidget* serialPortNameComponent=new QWidget;
	QHBoxLayout* serialPortNameLayout=new QHBoxLayout;
	serialPortNameComponent->setLayout(serialPortNameLayout);
	serialPortNameLayout->setSpacing(SERIAL_HORIZONTAL_SPACING);
	QLabel* serialPortNameLabel=new QLabel(SERIAL_PORT_NAME_LABEL_TEXT);
	serialPortNameLayout->addWidget(serialPortNameLabel);
	serialPortNameEdit=new QLineEdit;
	serialPortNameLayout->addWidget(serialPortNameEdit);

	serialPortChooserLayout->addWidget(serialPortNameComponent);

	serialPortList=new QListWidget;
	serialPortList->addItems(ExtendedSerialPort::getSerialPorts());
	connect(serialPortList, SIGNAL(currentTextChanged(const QString&)), this, SLOT(serialPortChanged(const QString&)));
	serialPortChooserLayout->addWidget(serialPortList);

	return serialPortChooser;
}

QWidget* WP34sFlashGui::buildDebugCheckBox()
{
	debugCheckBox=new QCheckBox("Debug");
	return debugCheckBox;
}

QWidget* WP34sFlashGui::buildButtons()
{
	QWidget* buttonsComponent=new QWidget;
	QHBoxLayout* buttonsLayout=new QHBoxLayout;
	buttonsComponent->setLayout(buttonsLayout);

	QPushButton* flashButton=new QPushButton("Flash");
	connect(flashButton, SIGNAL(clicked(bool)), this, SLOT(onFlashButtonClicked(bool)));
	buttonsLayout->addWidget(flashButton);
	QPushButton* exitButton=new QPushButton("Exit");
	connect(exitButton, SIGNAL(clicked(bool)), this, SLOT(onQuitButtonClicked(bool)));
	buttonsLayout->addWidget(exitButton);
	buttonsLayout->setSizeConstraint(QLayout::SetFixedSize);

	return buttonsComponent;
}

void WP34sFlashGui::serialPortChanged(const QString& aSerialPortName)
{
	serialPortNameEdit->setText(aSerialPortName);
}

void WP34sFlashGui::onFileDialogButtonClicked(bool checked)
{
	Q_UNUSED(checked)

	QString currentFilename=firmwareFilenameEdit->text();
	QString currentDirname= QDir::homePath();
	if(!currentFilename.trimmed().isEmpty())
	{
		QDir currentDir=QFileInfo(currentFilename).dir();
		if(currentDir.exists() && currentDir.isReadable())
		{
			currentDirname=currentDir.path();
		}
	}
	QString filename=QFileDialog::getOpenFileName(this, "Choose Firmware", currentDirname, "*.bin");
	if(!filename.isEmpty())
	{
		firmwareFilenameEdit->setText(filename);
	}
}

void WP34sFlashGui::onFlashButtonClicked(bool checked)
{
	Q_UNUSED(checked)

	WP34sFlash flash(firmwareFilenameEdit->text(), serialPortNameEdit->text(), debugCheckBox->isChecked());
	if(flash.isValid())
	{
		WP34sFlashDialog flashDialog(flash, this);
		flashDialog.exec();
	}
	else
	{
		QMessageBox::critical(this, "Cannot Flash", flash.errorMessage());
	}
}

void WP34sFlashGui::onQuitButtonClicked(bool checked)
{
	Q_UNUSED(checked)
	QApplication::quit();
}
