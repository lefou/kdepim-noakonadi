/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
#include <qfont.h>
#include <qstring.h>
#include <qfontmetrics.h>
#include <qpixmap.h>
#include <qstringlist.h>

// KDE includes
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "EmpathConfig.h"
#include "EmpathFolderListItem.h"
#include "EmpathFolder.h"
#include "EmpathMailbox.h"
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "Empath.h"

EmpathFolderListItem::EmpathFolderListItem(
		QListView * parent,
		const EmpathURL & url)
	:
		QListViewItem(parent),
		url_(url),
		tagged_(false)
{
	empathDebug("ctor with mailbox");

	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QStringList l = c->readListEntry(EmpathConfig::KEY_FOLDER_ITEMS_OPEN, ',');
	
	QStringList::ConstIterator it(l.begin());
	for (; it != l.end(); it++)
		if (*it == url_.asString())
			setOpen(true);


	EmpathMailbox * m(empath->mailbox(url_));
	
	if (m == 0) {
		empathDebug("Can't find the mailbox !!!!");
		return;
	}
	QObject::connect(m, SIGNAL(countUpdated(int, int)),
		this, SLOT(s_setCount(int, int)));
	
	setText(0, m->name());
	setText(1, QString().setNum(m->unreadMessageCount()));
	setText(2, QString().setNum(m->messageCount()));
	setPixmap(0, empathIcon(m->pixmapName()));
}

EmpathFolderListItem::EmpathFolderListItem(
		QListViewItem * parent,
		const EmpathURL & url)
	:
		QListViewItem(parent),
		url_(url),
		tagged_(false)
{
	empathDebug("ctor with folder \"" + url_.asString() + "\"");

	KConfig * c(kapp->getConfig());
	c->setGroup(EmpathConfig::GROUP_DISPLAY);

	QStringList l = c->readListEntry(EmpathConfig::KEY_FOLDER_ITEMS_OPEN);
	
	QStringList::ConstIterator it(l.begin());
	for (; it != l.end(); it++)
		if (*it == url_.asString())
			setOpen(true);

	EmpathMailbox * m(empath->mailbox(url_));
	
	EmpathFolder * f(empath->folder(url_));
	
	if (f == 0) {
		empathDebug("Can't find the folder !!!!");
		return;
	}

	QObject::connect(
		f,		SIGNAL(countUpdated(int, int)),
		this,	SLOT(s_setCount(int, int)));
	
	QObject::connect(
		this,	SIGNAL(update()),
		f,		SLOT(s_update()));
	
	QString s = url_.folderPath();
	if (s.right(1) == "/")
		s = s.remove(s.length(), 1);
	empathDebug("s now == " + s);
	s = s.right(s.length() - s.findRev("/") - 1);
	empathDebug("s now == " + s);
	
	setText(0, s);
	setPixmap(0, empathIcon(f->pixmapName()));
	setText(1, "...");
	setText(2, "...");
//	setText(1, QString().setNum(f->unreadMessageCount()));
//	setText(2, QString().setNum(f->messageCount()));
}
	
EmpathFolderListItem::~EmpathFolderListItem()
{
	empathDebug("dtor - my url was \"" + url_.asString() + "\"");
}

	QString
EmpathFolderListItem::key(int column, bool) const
{
	QString tmpString;
	
	if (url_.hasFolder()) {
	
		EmpathFolder * f(empath->folder(url_));
		
		if (f != 0)
			tmpString.sprintf("%08i", f->id());
	
	} else {
	
		EmpathMailbox * m(empath->mailbox(url_));
		
		if (m != 0)
			tmpString.sprintf("%08i", m->id());
	}

	return tmpString;
}

	void
EmpathFolderListItem::setup()
{
	empathDebug("setup() called");
	
	widthChanged();
	
	int th = QFontMetrics(empathGeneralFont()).height();
	
	if (!pixmap(0))
		setHeight(th);
	else 
		setHeight(QMAX(pixmap(0)->height(), th));
}

	void
EmpathFolderListItem::s_setCount(int unread, int read)
{
	setText(1, QString().setNum(unread));
	setText(2, QString().setNum(read));
}

	void
EmpathFolderListItem::setOpen(bool o)
{
	emit(opened());
	QListViewItem::setOpen(o);
}

	void
EmpathFolderListItem::s_update()
{
	emit(update());
}

