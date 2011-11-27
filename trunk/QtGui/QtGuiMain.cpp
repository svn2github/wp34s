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

#include <QtGui>
#include "QtEmulator.h"

int main(int argv, char **args)
{
	QApplication app(argv, args);
	QApplication::setOrganizationName(ORGANIZATION_NAME);
	QApplication::setApplicationName(APPLICATION_NAME);
	try
	{
		QtEmulator emulator;
		emulator.show();

		return app.exec();
	}
	catch(QtSkinException& exception)
	{
		QMessageBox messageBox;
		messageBox.setIcon(QMessageBox::Critical);
		messageBox.setText("Error reading skin, cannot continue");
		messageBox.setInformativeText(exception.what());
		messageBox.setStandardButtons(QMessageBox::Ok);
		messageBox.setDefaultButton(QMessageBox::Ok);
		messageBox.exec();
		return 1;
	}
}
