/* testactions			KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to sync actions.
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

#include "options.h"
#include "syncAction.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

bool run_modes(bool test, bool local)
{
	bool ok = true;

	kDebug() <<"***\n*** Sync Modes ("
		<< ( test ? "" : "no")
		<< "test, "
		<< ( local ? "" : "no")
		<< "local)\n***\n";


	for (int m = (int)SyncAction::SyncMode::eHotSync;
		m <= (int) SyncAction::SyncMode::eRestore ;
		++m)
	{
    kDebug() << "TEST:" << m;
		SyncAction::SyncMode mode((SyncAction::SyncMode::Mode)m,test,local);
		kDebug() <<"*" << mode.name();
		SyncAction::SyncMode mode2(mode.list());
		if (!(mode==mode2)) {
			kDebug() <<"E" <<"Modes mismatch [" << mode.name() <<"] ["
				<< mode2.name() << "]";
			ok = false;
		}
	}

	return ok;
}

bool single_mode(int m, bool test, bool local)
{
	SyncAction::SyncMode mode((SyncAction::SyncMode::Mode)m,test,local);

	kDebug() <<"*" << m << test << local;

	if ((mode.mode() == m) && (mode.isTest() == test) && (mode.isLocal() == local))
	{
		return true;
	}
	else
	{
		kDebug() <<"E" <<"Modes mismatch" << m << test << local
			<< "[" << mode.name() << "]";
		return false;
	}
}

int main(int argc, char **argv)
{
  FUNCTIONSETUP;

  KAboutData aboutData("testactions", 0,ki18n("Test Actions"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);
  KApplication app( false );
  
	if (!run_modes(false,false)) return 1;
	if (!run_modes(false,true)) return 1;
	if (!run_modes(true,false)) return 1;
	if (!run_modes(true,true)) return 1;

	kDebug() <<"***\n*** Sync Modes - misc\n***";
	if (!single_mode(3,false,false)) return 1;
	if (!single_mode(1,true,true)) return 1;

	return 0;
}


