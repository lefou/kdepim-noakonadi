/***************************************************************************
                          kmailcvt.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmailcvt.h"
#include <kaboutapplication.h>
#include <qpushbutton.h>

#include "filters.hxx"

KMailCVT::KMailCVT(QWidget *parent, const char *name)
	: KWizard(parent, name, true) {

	_parent = parent;
	setCaption( i18n( "KMailCVT Import Tool" ) );

	selfilterpage = new KSelFilterPage(this);
	addPage( selfilterpage, i18n( "Step 1: Select filter" ) );

	importpage = new KImportPage(this);
	addPage( importpage, i18n( "Step 2: Importing..." ) );
	setFinishEnabled(QWizard::page(2), false);

	filters = Filter::createFilters();
	for ( Filter* filter = filters.first(); filter; filter = filters.next() )
		selfilterpage->addFilter( filter );
}

KMailCVT::~KMailCVT() {
}

void KMailCVT::next() {
	if(currentPage()==selfilterpage){
		// Save selected filter
		Filter *selectedFilter = selfilterpage->getSelectedFilter();
		// Goto next page
		QWizard::next();
		// Disable back
		setBackEnabled(QWizard::currentPage(), false);
		// Start import
		FilterInfo *info = new FilterInfo(importpage, _parent);
		info->setStatusMsg(i18n("Import in progress"));
		info->clear(); // Clear info from last time
		selectedFilter->import(info);
		info->setStatusMsg(i18n("Import finished"));
		// Cleanup
		delete info;
		// Enable finish button also reenable back
		setFinishEnabled(QWizard::currentPage(), true);
		setBackEnabled(QWizard::currentPage(), true);
		return;
	}
	QWizard::next();
}

void KMailCVT::reject() {
	if ( currentPage() == importpage ) FilterInfo::terminateASAP();
        else KWizard::reject();
}

void KMailCVT::help()
{
	KAboutData aboutData( "kmailcvt", I18N_NOOP("KMailCVT"),
		KMAILCVT_VERSION, KMAILCVT, 
		KAboutData::License_GPL_V2,
		"(c) 2000-3, The KMailCVT developers");
	aboutData.addAuthor("Hans Dijkema","Original author", "kmailcvt@hum.org", "http://www.hum.org/kmailcvt.html");
	aboutData.addAuthor("Laurence Anderson","New GUI & cleanups", "l.d.anderson@warwick.ac.uk");
	for ( Filter* filter = filters.first(); filter; filter = filters.next() )
		aboutData.addAuthor(filter->author().latin1(), filter->name().latin1());

	KAboutApplication a(&aboutData, _parent);
	a.exec();
}

#include "kmailcvt.moc"
