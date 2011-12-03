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

QtPreferencesDialog::QtPreferencesDialog(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName, QWidget* aParent)
: QDialog(aParent)
{
	setWindowTitle(PREFERENCES_TITLE);

	QVBoxLayout* dialogLayout=new QVBoxLayout;

	useCustomDirectoryButton=new QRadioButton(USE_CUSTOM_DIRECTORY_TEXT);
	connect(useCustomDirectoryButton, SIGNAL(toggled(bool)), this, SLOT(customDirectoryToggled(bool)));
	dialogLayout->addWidget(useCustomDirectoryButton);

	QHBoxLayout* directoryLayout=new QHBoxLayout;
	directoryNameEdit=new QLineEdit;
	int minimumWidth=directoryNameEdit->fontMetrics().width(DIRECTORY_NAME_DEFAULT_CHAR)*DIRECTORY_NAME_DEFAULT_WIDTH;
	directoryNameEdit->setMinimumWidth(minimumWidth);
	directoryLayout->addWidget(directoryNameEdit);

	chooseButton=new QPushButton(CHOOSE_DIRECTORY_TEXT);
	directoryLayout->addWidget(chooseButton);
	dialogLayout->addItem(directoryLayout);
	connect(chooseButton, SIGNAL(clicked(bool)), this, SLOT(chooseDirectory()));

	QDialogButtonBox* buttonBox=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	dialogLayout->addWidget(buttonBox);

	dialogLayout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(dialogLayout);

	useCustomDirectoryButton->setChecked(aCustomDirectoryActiveFlag);
	directoryNameEdit->setText(aCustomDirectoryName);
	customDirectoryToggled(isCustomDirectoryActive());
}

QtPreferencesDialog::~QtPreferencesDialog()
{
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
