/* abbrowser-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the abbrowser-conduit plugin.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#if KDE_VERSION < 300
#include <kapp.h>
#else
#include <kapplication.h>
#endif
#include <kinstance.h>
#include <kaboutdata.h>

#include <dcopclient.h>

#include <time.h> // Needed by pilot-link include

#include <pi-memo.h>

#include "abbrowser-conduit.h"
#include "abbrowser-setup.h"

#include "abbrowser-factory.moc"


extern "C"
{

void *init_libabbrowserconduit()
{
	return new AbbrowserConduitFactory;
}

} ;


/* static */ KAboutData *AbbrowserConduitFactory::fAbout = 0L;
/* static */ const char *AbbrowserConduitFactory::fGroup = "Abbrowser-conduit";
/* static */ const char *AbbrowserConduitFactory::fStreetType = "PilotStreet";
/* static */ const char *AbbrowserConduitFactory::fSmartMerge = "SmartMerge";
/* static */ const char *AbbrowserConduitFactory::fResolution = "ConflictResolve";
/* static */ const char *AbbrowserConduitFactory::fOtherMap = "PilotOther";


AbbrowserConduitFactory::AbbrowserConduitFactory(QObject *p, const char *n) :
	KLibFactory(p,n)
{
	FUNCTIONSETUP;

	fInstance = new KInstance("abbrowserconduit");
	fAbout = new KAboutData("abbrowserconduit",
		I18N_NOOP("Abbrowser Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the Abbrowser Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2001, Dan Pilone");
	fAbout->addAuthor("Greg Stern",
		I18N_NOOP("Primary Author"));
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addCredit("David Bishop",
		I18N_NOOP("UI"));
}

AbbrowserConduitFactory::~AbbrowserConduitFactory()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *AbbrowserConduitFactory::createObject( QObject *p,
	const char *n,
	const char *c,
	const QStringList &a)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Creating object of class "
		<< c
		<< endl;
#endif

	if (qstrcmp(c,"ConduitConfig")==0)
	{
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w)
		{
			return new AbbrowserWidgetSetup(w,n,a);
		}
		else 
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast parent to widget."
				<< endl;
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0)
	{
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d)
		{
			return new AbbrowserConduit(d,n,a);
		}
		else
		{
			kdError() << k_funcinfo
				<< ": Couldn't cast parent to KPilotDeviceLink"
				<< endl;
			return 0L;
		}
	}

	return 0L;
}


// $Log$
// Revision 1.1  2001/10/31 23:54:45  adridg
// CVS_SILENT: Ongoing conduits ports
//
