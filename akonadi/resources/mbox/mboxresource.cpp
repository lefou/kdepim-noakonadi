/*
    Copyright (c) 2009 Bertjan Broeksem <b.broeksema@kdemail.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mboxresource.h"

#include <akonadi/attributefactory.h>
#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchscope.h>
#include <boost/shared_ptr.hpp>
#include <kmime/kmime_message.h>
#include <KWindowSystem>
#include <QtDBus/QDBusConnection>

#include "configdialog.h"
#include "deleteditemsattribute.h"
#include "mbox.h"
#include "settings.h"
#include "settingsadaptor.h"

using namespace Akonadi;

typedef boost::shared_ptr<KMime::Message> MessagePtr;

static QString mboxFile(const QString &remoteItemId)
{
  return remoteItemId.left(remoteItemId.lastIndexOf(QDir::separator()));
}

static quint64 itemOffset(const QString &remoteItemId)
{
  return remoteItemId.split(QDir::separator()).last().toULongLong();
}

MboxResource::MboxResource( const QString &id ) : ResourceBase( id )
{
  new SettingsAdaptor( Settings::self() );
  QDBusConnection::sessionBus().registerObject( QLatin1String( "/Settings" ),
                              Settings::self(), QDBusConnection::ExportAdaptors );

  // Register the list of deleted items as an attribe of the collection.
  AttributeFactory::registerAttribute<DeletedItemsAttribute>();

  changeRecorder()->fetchCollection( true );
  changeRecorder()->itemFetchScope().fetchFullPayload( true );
}

MboxResource::~MboxResource()
{
}

void MboxResource::configure( WId windowId )
{
  ConfigDialog dlg;
  if ( windowId )
    KWindowSystem::setMainWindow( &dlg, windowId );
  dlg.exec();

  synchronizeCollectionTree();
}

void MboxResource::retrieveCollections()
{
  MBox mbox(Settings::self()->file());

  QString errMsg;
  if ( !mbox.isValid( errMsg ) ) {
    emit error( errMsg );
    collectionsRetrieved( Collection::List() );
  }

  Collection col;
  col.setParent(Collection::root());
  col.setRemoteId(Settings::self()->file());
  col.setName(name());

  QStringList mimeTypes;
  mimeTypes << "message/rfc822" << Collection::mimeType();
  col.setContentMimeTypes( mimeTypes );

  collectionsRetrieved(Collection::List() << col);
}

void MboxResource::retrieveItems( const Akonadi::Collection &col )
{
  MBox mbox( col.remoteId() );

  if (Settings::self()->lockfileMethod() == Settings::procmail)
    mbox.setProcmailLockFile(Settings::self()->lockfile());

  if (!mbox.isValid()) {
    emit error( i18n("Invalid mbox file: %1", col.remoteId() ) );
    itemsRetrieved(Item::List());
    return;
  }

  if (int rc = mbox.open() != 0) { // This can happen for example when the lock fails.
    emit error(i18n("Error while opening mbox file %1: %2", col.remoteId(), rc));
    itemsRetrieved(Item::List());
    return;
  }

  QList<MsgInfo> entryList;
  if (col.hasAttribute<DeletedItemsAttribute>()) {
    DeletedItemsAttribute *attr = col.attribute<DeletedItemsAttribute>();
    entryList = mbox.entryList(attr->deletedItemOffsets());
  } else { // No deleted items (yet)
    entryList = mbox.entryList();
  }

  Item::List items;
  int offset = 0;
  QString colId = col.remoteId();
  foreach (const MsgInfo &entry, entryList) {
    // TODO: Use cache policy to see what actualy has to been set as payload.
    //       Currently most views need a minimal amount of information so the
    //       Items get Envelopes as payload.
    KMime::Message *mail = new KMime::Message();
    mail->setHead(KMime::CRLFtoLF(mbox.readEntryHeaders(entry.first)));
    mail->parse();

    Item item;
    item.setRemoteId(colId + QDir::separator() + QString::number(offset));
    item.setMimeType("message/rfc822");
    item.setSize(entry.second);
    item.setPayload(MessagePtr(mail));
    items << item;

    offset += entry.second;
  }

  mbox.close(); // Now we have the items, unlock and close the mbox file.

  itemsRetrieved( items );
}

bool MboxResource::retrieveItem( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED(parts);

  MBox mbox(mboxFile(item.remoteId()));

  if(!mbox.isValid())
    return false;

  mbox.open();

  quint64 offset = itemOffset(item.remoteId());
  const QByteArray rawMsg = mbox.readEntry(offset);

  Q_ASSERT(rawMsg.size() == item.size());

  KMime::Message *mail = new KMime::Message();
  mail->setContent(KMime::CRLFtoLF(rawMsg));
  mail->parse();

  Item i(item);
  i.setPayload(MessagePtr(mail));
  itemRetrieved(i);
  return true;
}

void MboxResource::aboutToQuit()
{
}

void MboxResource::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  Q_UNUSED(item);
  Q_UNUSED(collection);
  changeProcessed();
}

void MboxResource::itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED(item);
  Q_UNUSED(parts);
  changeProcessed();
}

void MboxResource::itemRemoved( const Akonadi::Item &item )
{
  Q_UNUSED(item);
  changeProcessed();
}


AKONADI_RESOURCE_MAIN( MboxResource )

#include "mboxresource.moc"
