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
#include <qdir.h>
#include <qstring.h>
#include <qdir.h>
#include <qlist.h>

// Local includes
#include "EmpathEnum.h"
#include "EmpathURL.h"
#include "EmpathDefines.h"
#include "RMM_Enum.h"
#include "RMM_Envelope.h"
#include "RMM_Message.h"
#include "RMM_MessageID.h"

class EmpathFolder;

class EmpathMaildir
{
	public:
		
		EmpathMaildir()
		{ empathDebug("default ctor"); }

		EmpathMaildir(const QString & basePath, const EmpathURL & url);
		virtual ~EmpathMaildir();
		
		const QString &		basePath()	const { return basePath_; }
		const EmpathURL &	url()		const { return url_; }
		const QString &		path()		const { return path_; }
		
		void		mark(const EmpathURL & message, MessageStatus msgStat);
		
		bool		writeMessage(const RMessage & msg);
		
		Q_UINT32				sizeOfMessage			(const QString & id);
		QString					plainBodyOfMessage		(const QString & id);
		REnvelope *				envelopeOfMessage		(const QString & id);
		RMessage *				message					(const QString & id);
		bool					removeMessage			(const QString & id);
		RMessage::MessageType	typeOfMessage			(const QString & id);
		
		const char * className() const { return "EmpathMaildir"; }
		
		void sync(const EmpathURL & url);
		
	private:
		
		QString		_write(const RMessage & msg);
		QCString	_messageData(const QString & filename);
		void		_init();
		void		_markNewMailAsSeen();
		void 		_markAsSeen(const QString & name);
		void		_clearTmp();
		bool		_setupDirs();
		QString		_generateUnique();
		QString		_generateFlagsString(MessageStatus s);
		void		_readIndex();
		void		_writeIndex();
		
		// Order dependency
		Q_UINT32	seq_;
		QString		path_;
		QString		basePath_;
		EmpathURL	url_;
		// End order dependency
		
		QDir d;
		
		QDateTime	mtime_;
};

typedef QList<EmpathMaildir> EmpathMaildirList;
typedef QListIterator<EmpathMaildir> EmpathMaildirListIterator;

