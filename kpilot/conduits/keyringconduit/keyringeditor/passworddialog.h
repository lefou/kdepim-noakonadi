#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H
/* passworddialog.h			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "ui_passworddialog.h"

class KeyringHHDataProxy;
class KeyringListModel;

class PasswordDialog : public QDialog
{
	Q_OBJECT
public:
	
	enum Mode
	{
		New = 0,
		Normal
	};

	/**
	 * Returns the entered password or QString() when canceled. Use mode to
	 * generate a new password or just to get a password from the user.
	 */
	static QString getPassword( QWidget* parent = 0, Mode mode = New );

protected:
	PasswordDialog( QWidget *parent = 0, Mode mode = New );

private slots:
	/**
	 * Only accept when the passwords are equal.
	 */
	void accept();
	
	void checkPasswords();
	
	void checkPassword();

private:
	Ui::PasswordDialog fUi;
	Mode fMode;
};

#endif