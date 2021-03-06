/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "groupwaredataadaptor.h"
#include <kdebug.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kresources/idmapper.h>

using namespace KPIM;

GroupwareUploadItem::GroupwareUploadItem( UploadType type ) : mItemType( KPIM::FolderLister::Unknown ), mType( type )
{
}

KUrl GroupwareUploadItem::adaptNewItemUrl( GroupwareDataAdaptor *adaptor,
                                           const KUrl &baseurl )
{
kDebug()<<"GroupwareUploadItem::adaptNewItemUrl, baseurl=" << baseurl.url();
  if ( adaptor ) {
    QString path( adaptor->defaultNewItemName( this ) );
kDebug() <<"path=" << path;
    KUrl u( baseurl );
    if ( path.isEmpty() ) return u;
    else {
      u.addPath( path );
kDebug() <<"Final Path for new item:" << u.url();
      return u;
    }
  } else return baseurl;
}

KIO::TransferJob *GroupwareUploadItem::createRawUploadJob(
                       GroupwareDataAdaptor *adaptor, const KUrl &/*baseurl*/ )
{
  Q_ASSERT( adaptor );
  if ( !adaptor ) return 0;
  const QString dta = data();
  //kDebug(7000) <<"Uploading:" << data;
  KUrl upUrl( url() );
  if ( adaptor )
    adaptor->adaptUploadUrl( upUrl );
  kDebug(7000) <<"Uploading to:" << upUrl.prettyUrl();
  KIO::TransferJob *job = KIO::storedPut( dta.toUtf8(), upUrl, -1, KIO::Overwrite | 
                                          KIO::HideProgressInfo );
  job->addMetaData( "PropagateHttpHeader", "true" );
  if ( adaptor ) {
    job->addMetaData( "customHTTPHeader", "Content-Type: " + adaptor->mimeType() );
  }
  return job;
}

KIO::TransferJob *GroupwareUploadItem::createUploadNewJob(
      GroupwareDataAdaptor *adaptor, const KUrl &baseurl )
{
kDebug()<<"GroupwareUploadItem::createUploadNewJob, baseurl=" << baseurl.url();
  setUrl( adaptNewItemUrl( adaptor, baseurl ) );
  KIO::TransferJob *job = createRawUploadJob( adaptor, baseurl );
  if ( job ) {
    kDebug() <<"Adding If-None-Match";
    QString header;
    if ( job->outgoingMetaData().contains("customHTTPHeader") ) {
      header = job->outgoingMetaData()["customHTTPHeader"];
      header += "\r\n";
    }
    header += "If-None-Match: *";
    job->addMetaData( "customHTTPHeader", header );
  }
  return job;
}

KIO::TransferJob *GroupwareUploadItem::createUploadJob(
                           GroupwareDataAdaptor *adaptor, const KUrl &baseurl )
{
kDebug()<<"GroupwareUploadItem::createUploadJob";
  KIO::TransferJob *job = createRawUploadJob( adaptor, baseurl );
  if ( job && adaptor ) {
  kDebug()<<"Adding If-Match header:" << adaptor->idMapper()->fingerprint( uid() );
    QString header;
    if ( job->outgoingMetaData().contains("customHTTPHeader") ) {
      header = job->outgoingMetaData()["customHTTPHeader"];
      header += "\r\n";
    }
kDebug()<<"old HEADER:" << header;
    header += "If-Match: " + adaptor->idMapper()->fingerprint( uid() );
kDebug()<<"new HEADER:" << header;
    job->addMetaData( "customHTTPHeader", header );
  }
  return job;
}


GroupwareDataAdaptor::GroupwareDataAdaptor()
  : QObject(), mFolderLister( 0 ), mIdMapper( 0 )
{
}

GroupwareDataAdaptor::~GroupwareDataAdaptor()
{
}

void GroupwareDataAdaptor::setUserPassword( KUrl &url )
{
  kDebug(5800) <<"GroupwareDataAdaptor::setUserPassword, mUser="
                << mUser;
  url.setUser( mUser );
  url.setPass( mPassword );
}

FolderLister::Entry::List GroupwareDataAdaptor::defaultFolders()
{
  return FolderLister::Entry::List();
}

KIO::TransferJob *GroupwareDataAdaptor::createUploadJob( const KUrl &url,
                                                     GroupwareUploadItem *item )
{
  if ( item ) {
    KIO::TransferJob *job = item->createUploadJob( this, url );
    setUidForJob( job, item->uid() );
    return job;
  } else return 0;
}

KIO::TransferJob *GroupwareDataAdaptor::createUploadNewJob( const KUrl &url,
                                                     GroupwareUploadItem *item )
{
kDebug()<<"GroupwareDataAdaptor::createUploadNewJob, url=" << url.url();
  if ( item ) {
    KIO::TransferJob *job = item->createUploadNewJob( this, url );
    setUidForJob( job, item->uid() );
    return job;
  } else return 0;
}

void GroupwareDataAdaptor::processDownloadListItem( const KUrl &entry,
        const QString &newFingerprint, KPIM::FolderLister::ContentType type )
{
  bool download = false;
  const QString &location = entry.path();

  emit itemOnServer( entry );
  // if not locally present, download
  const QString &localId = idMapper()->localId( location );
  kDebug(5800) <<"Looking up remote:" << location
                << "found:" << localId;
  if ( localId.isEmpty() || !localItemExists( localId ) ) {
    //kDebug(7000) <<"Not locally present, download:" << location;
    download = true;
  } else {
    kDebug(5800) <<"Locally present";
    // locally present, let's check if it's newer than what we have
    const QString &oldFingerprint = idMapper()->fingerprint( localId );
      if ( oldFingerprint != newFingerprint ) {
      kDebug(5800) <<"Fingerprint changed old:" << oldFingerprint <<
        "new:" << newFingerprint;
      // something changed on the server, check if we also changed it locally
      if ( localItemHasChanged( localId ) ) {
        // TODO conflict resolution
        kDebug(5800) <<"TODO conflict resolution";
        download = true;
      } else {
        download = true;
      }
    } else {
      kDebug(5800) <<"Fingerprint not changed, don't download this";
    }
  }
  if ( download ) {
    emit itemToDownload( entry, type );
  }
}

bool GroupwareDataAdaptor::interpretRemoveJob( KIO::Job *job, const QString &/*jobData*/ )
{
  if ( !job ) return false;
  KIO::DeleteJob *deljob = dynamic_cast<KIO::DeleteJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( deljob ) {
    KUrl::List urls( deljob->urls() );
    for ( KUrl::List::Iterator it = urls.begin(); it != urls.end(); ++it ) {
      if ( error ) {
        emit itemDeletionError( *it, err );
      } else {
        // FIXME: Don't use QString() here
        emit itemDeleted( QString(), *it );
      }
    }
    return true;
  } else {
    return false;
  }
}


bool GroupwareDataAdaptor::interpretUploadJob( KIO::Job *job, const QString &/*jobData*/ )
{
  kDebug(7000) <<"GroupwareDataAdaptor::interpretUploadJob";
  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( trfjob ) {
    KUrl url( trfjob->url() );
    if ( error ) {
      emit itemUploadError( url, err );
    } else {
      // We don't know the local id here (and we don't want to extract it from
      // the idMapper, that's the task of the receiver
      emit itemUploaded( uidFromJob( job ), url );
    }
    return true;
  } else {
    return false;
  }
}

bool GroupwareDataAdaptor::interpretUploadNewJob( KIO::Job *job, const QString &/*jobData*/ )
{
// TODO: How does the incidence mapper know the old/new ids???
  kDebug(7000) <<"GroupwareDataAdaptor::interpretUploadNewJob";
  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  bool error = job->error();
  const QString err = job->errorString();

  if ( trfjob ) {
    KUrl url( trfjob->url() );
    if ( error ) {
      emit itemUploadNewError( idMapper()->localId( url.path() ), err );
    } else {
      // We don't know the local id here (and we don't want to extract it from
      // the idMapper, that's the task of the receiver
      emit itemUploadedNew( uidFromJob( job ), url );
    }
    return true;
  } else {
    return false;
  }
}

QString GroupwareDataAdaptor::uidFromJob( KIO::Job *job ) const
{
kDebug()<<"GroupwareDataAdaptor::uidFromJob("<<job<<")";
  if ( mJobUIDMap.contains( job ) ) {
    kDebug()<<"  Contained:"<< mJobUIDMap[job];
    return mJobUIDMap[ job ];
  } else {
    return QString();
  }
}

void GroupwareDataAdaptor::setUidForJob( KIO::Job *job, const QString &uid )
{
  if ( uid.isEmpty() ) {
    mJobUIDMap.remove( job );
  } else {
    mJobUIDMap[ job ] = uid;
  }
}


#include "groupwaredataadaptor.moc"
