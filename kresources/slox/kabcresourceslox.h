/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef KABC_RESOURCESLOX_H
#define KABC_RESOURCESLOX_H

#include "webdavhandler.h"

#include <kabc/resource.h>

#include <qmap.h>
#include <qdom.h>

namespace KIO {
class DavJob;
class Job;
}

class KConfig;

namespace KABC {

class ResourceSlox : public Resource
{
    Q_OBJECT
  public:
    ResourceSlox( const KConfig * );
    ResourceSlox( const KURL &url,
                  const QString &user, const QString &password );
    ~ResourceSlox();

    virtual void writeConfig( KConfig * );

    virtual bool doOpen();
    virtual void doClose();

    virtual Ticket *requestSaveTicket();
    virtual void releaseSaveTicket( Ticket* );

    virtual bool load();
    virtual bool asyncLoad();
    virtual bool save( Ticket * );
    virtual bool asyncSave( Ticket * );

    virtual void insertAddressee( const Addressee &addr );
    virtual void removeAddressee( const Addressee& addr );

    void setURL( const KURL &url );
    KURL url() const;

    void setUser( const QString &user );
    QString user() const;

    void setPassword( const QString &password );
    QString password() const;

  protected:
    void init( const KURL &url,
               const QString &user, const QString &password );

    void parseContactAttribute( const QDomElement &e, Addressee &a );

  protected slots:
    void slotResult( KIO::Job *job );

  private:
    KURL mURL;
    QString mUser;
    QString mPassword;

    KIO::DavJob *mDownloadJob;

    WebdavHandler mWebdavHandler;
};

}

#endif
