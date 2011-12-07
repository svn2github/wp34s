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
#define USE_CUSTOM_DIRECTORY_TEXT "Use a custom directory for memory files"
#define CHOOSE_DIRECTORY_TEXT "Choose"

#define DIRECTORY_NAME_DEFAULT_WIDTH 60
#define DIRECTORY_NAME_DEFAULT_CHAR 'x'

class QtPreferencesDialog: public QDialog
{
	Q_OBJECT

public:
	QtPreferencesDialog(bool aCustomDirectoryActiveFlag, const QString& aCustomDirectoryName, QWidget* aParent=NULL);
	~QtPreferencesDialog();

public:
	bool isCustomDirectoryActive() const;
	QString getCustomDirectoryName() const;

public slots:
	void customDirectoryToggled(bool);
	void chooseDirectory();

private:
	QRadioButton* useCustomDirectoryButton;
	QLineEdit* directoryNameEdit;
	QPushButton* chooseButton;
};

#endif /* QTPREFERENCESDIALOG_H_ */
