/*
    This file is part of KMail.

    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 - 2004 Bo Thorsen <bo@sonofthor.dk>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

/** @file This file contains the class KMailICalIfaceImpl which actually
* implements the ICal (DCOP) interface for KMail.
*/

#ifndef KMAILICALIFACEIMPL_H
#define KMAILICALIFACEIMPL_H

#include <QPointer>
#include <QMap>
#include <QList>
#include <QHash>

#include <kurl.h>

#include <libkdepim/foldertreewidget.h>

#include "kmfoldertype.h"
#include "groupware_types.h"

class KMFolder;
class KMMessage;
class KMFolderDir;

namespace KMail {

  // Local helper class
class ExtraFolder {
public:
  ExtraFolder( KMFolder* f );
  ~ExtraFolder();
  QPointer<KMFolder> folder;
};

class Accumulator {
public:
  Accumulator( const QString& t, const QString& f, int c )
  :type( t ), folder( f ), count( c ) {}

  void add( const QString& incidence ) {
    incidences << incidence;
    count--;
  }
  bool isFull() { return count == 0; }

  const QString type;
  const QString folder;
  QStringList incidences;
  int count;
};

}

/** The implementation of the interface. */
class KMailICalIfaceImpl :public QObject {
  Q_OBJECT
public:
  KMailICalIfaceImpl();
  ~KMailICalIfaceImpl();

  /**
   * This registers the interface on the D-Bus session bus.
   * This is not done automatically when KMailICalIfaceImpl is created,
   * as we might not be ready for D-Bus calls during the initialization phase.
   * However, KMail itself uses the interface during initialization, for example
   * in KMFolderCachedImap::readConfig, so it needs to be ready before it is
   * registered at D-Bus.
   */
  void registerWithDBus();

  bool isWritableFolder( const QString& type, const QString& resource );

  KMail::StorageFormat storageFormat( const QString &resource );

  /// Update a kolab storage entry.
  /// If message is not there, it is added and
  /// given the subject as Subject: header.
  /// Returns the new mail serial number,
  /// or 0 if something went wrong,
  quint32 update( const QString& resource,
                   quint32 sernum,
                   const QString& subject,
                   const QString& plainTextBody,
                   const KMail::CustomHeader::List& customHeaders,
                   const QStringList& attachmentURLs,
                   const QStringList& attachmentMimetypes,
                   const QStringList& attachmentNames,
                   const QStringList& deletedAttachments );

  bool deleteIncidenceKolab( const QString& resource,
                             quint32 sernum );
  int incidencesKolabCount( const QString& mimetype,
                            const QString& resource );
  KMail::SernumDataPair::List incidencesKolab( const QString& mimetype,
                                           const QString& resource,
                                           int startIndex,
                                           int nbMessages );

  QList<KMail::SubResource> subresourcesKolab( const QString& contentsType );

  bool triggerSync( const QString& contentsType );

  // "Get" an attachment. This actually saves the attachment in a file
  // and returns a URL to it
  QString getAttachment( const QString& resource,
                      quint32 sernum,
                      const QString& filename );

  QString attachmentMimetype( const QString &resource,
                              quint32 sernum,
                              const QString &filename );

  QStringList listAttachments( const QString &resource, quint32 sernum );


  bool removeSubresource( const QString& );

  bool addSubresource( const QString& resource,
                       const QString& parent,
                       const QString& contentsType );

  // tell KOrganizer about messages to be deleted
  void msgRemoved( KMFolder*, KMMessage* );

  /** Initialize all folders. */
  void initFolders();

  /** Disconnect all slots and close the dirs. */
  void cleanup();

  /**
   * Returns true if resource mode is enabled and folder is one of the
   * resource folders.
   */
  bool isResourceFolder( KMFolder* folder ) const;

  /* Returns true if the folder is one of the standard resource folders, as
   * opposed to an extra folder. */
  bool isStandardResourceFolder( KMFolder* folder ) const;

  /**
   * Returns true if isResourceFolder( folder ) returns true, and
   * imap folders should be hidden.
   */
  bool hideResourceFolder( KMFolder* folder ) const;

  /**
   * Returns true if the given folder is the root of the groupware account,
   * groupware folders are hidden, and only groupware folders shown in this
   * account.
   */
  bool hideResourceAccountRoot( KMFolder* folder ) const;

  /**
   * Returns the resource folder type. Other is returned if resource
   * isn't enabled or it isn't a resource folder.
   */
  KPIM::FolderTreeWidgetItem::FolderType folderType( KMFolder* folder ) const;

  /**
   * Returns the name of the standard icon for a folder of given type or
   * QString() if the type is no groupware type.
   */
  QString folderPixmap( KPIM::FolderTreeWidgetItem::FolderType type ) const;

  /** Returns the localized name of a folder of given type.
   */
  QString folderName( KPIM::FolderTreeWidgetItem::FolderType type, int language = -1 ) const;

  /** Get the folder that holds *type* entries */
  KMFolder* folderFromType( const QString& type, const QString& folder );

  /** Return the ical type of a folder */
  QString icalFolderType( KMFolder* folder ) const;

  /** Find message matching a given UID. */
  KMMessage* findMessageByUID( const QString& uid, KMFolder* folder );
  /** Find message matching a given serial number. */
  static KMMessage* findMessageBySerNum( quint32 serNum, KMFolder* folder );

  /** Convenience function to delete a message. */
  void deleteMsg( KMMessage* msg );

  bool isEnabled() const { return mUseResourceIMAP; }

  /** Called when a folders contents have changed */
  void folderContentsTypeChanged( KMFolder*, KMail::FolderContentsType );

  /// @return the storage format of a given folder
    KMail::StorageFormat storageFormat( KMFolder* folder ) const;
  /// Set the storage format of a given folder. Called when seeing the kolab annotation.
  void setStorageFormat( KMFolder* folder, KMail::StorageFormat format );


  static const char* annotationForContentsType( KMail::FolderContentsType type );

  // Called after a folder was synced with the server
  void folderSynced( KMFolder* folder, const KUrl& folderURL );
  // Called when deletion of a folder from the server suceeded,
  // triggers fb re-generation
  void folderDeletedOnServer( const KUrl& folderURL );
  void addFolderChange( KMFolder* folder, KMail::FolderChanges changes );

  // See CachedImapJob::slotPutMessageResult
  bool isResourceQuiet() const;
  void setResourceQuiet(bool q);
  static QMap<QString, QString>* getResourceMap() { return mSubResourceUINamesMap; }

public slots:
  /* (Re-)Read configuration file */
  void readConfig();
  void slotFolderRemoved( KMFolder* folder );

  void slotIncidenceAdded( KMFolder* folder, quint32 sernum );
  void slotIncidenceDeleted( KMFolder* folder, quint32 sernum );
  void slotRefresh( const QString& type);

  // Called when a folder is made readonly or readwrite, or renamed,
  // or any other similar change that affects the resources
  void slotFolderPropertiesChanged( KMFolder* folder );
  void changeResourceUIName( const QString &folderPath, const QString &newName );

private slots:
  void slotRefreshFolder( KMFolder* );
  void slotCheckDone();
  void slotFolderLocationChanged( const QString&, const QString& );
  void slotFolderRenamed();
  void slotMessageRetrieved( KMMessage* );

signals:
  void incidenceAdded( const QString& type, const QString& folder,
                       uint sernum, int format, const QString& entry );
  void asyncLoadResult( const QMap<quint32, QString>, const QString& type,
                        const QString& folder );
  void incidenceDeleted( const QString& type, const QString& folder,
                         const QString& uid );
  void signalRefresh( const QString& type, const QString& folder );
  void subresourceAdded( const QString& type, const QString& resource,
                         const QString& label, bool writable, bool alarmRelevant );
  void subresourceDeleted( const QString& type, const QString& resource );

private:
  /** Helper function for initFolders. Initializes a single folder. */
  KMFolder* initFolder( KMail::FolderContentsType contentsType );
  KMFolder* initScalixFolder( KMail::FolderContentsType contentsType );

  void connectFolder( KMFolder* folder );

  KMFolder* extraFolder( const QString& type, const QString& folder );

  void syncFolder( KMFolder* folder ) const;

  struct StandardFolderSearchResult
  {
    enum FoundEnum { FoundAndStandard, NotFound, FoundByType, FoundByName };
    StandardFolderSearchResult() : folder( 0 ) {}
    StandardFolderSearchResult( KMFolder* f, FoundEnum e ) : folder( f ), found( e ) {}
    StandardFolderSearchResult( const QList<KMFolder*> &f, FoundEnum e ) :
        folder( f.first() ), folders( f ), found( e ) {}
    KMFolder* folder; // NotFound implies folder==0 of course.
    QList<KMFolder*> folders; // in case we found multiple default folders (which should not happen)
    FoundEnum found;
  };

  StandardFolderSearchResult findStandardResourceFolder( KMFolderDir* folderParentDir, KMail::FolderContentsType contentsType );
  KMFolder* findResourceFolder( const QString& resource );


  bool updateAttachment( KMMessage& msg,
                         const QString& attachmentURL,
                         const QString& attachmentName,
                         const QString& attachmentMimetype,
                         bool lookupByName );
  bool deleteAttachment( KMMessage& msg,
                         const QString& attachmentURL );
  quint32 addIncidenceKolab( KMFolder& folder,
                              const QString& subject,
                              const QString& plainTextBody,
                              const KMail::CustomHeader::List& customHeaders,
                              const QStringList& attachmentURLs,
                              const QStringList& attachmentNames,
                              const QStringList& attachmentMimetypes );
  static bool kolabXMLFoundAndDecoded( const KMMessage& msg, const QString& mimetype, QString& s );

  void handleFolderSynced( KMFolder* folder,
                           const KUrl& folderURL,
                           int _changes );
  void triggerKolabFreeBusy( const KUrl& folderURL );

  KMail::StorageFormat globalStorageFormat() const;

  static bool folderIsAlarmRelevant( const KMFolder * );

private:
  QPointer<KMFolder> mContacts;
  QPointer<KMFolder> mCalendar;
  QPointer<KMFolder> mNotes;
  QPointer<KMFolder> mTasks;
  QPointer<KMFolder> mJournals;

  // The extra IMAP resource folders
  // Key: folder location. Data: folder.
  QHash<QString, KMail::ExtraFolder*> mExtraFolders;
  // used for collecting incidences during async loading
  QHash<QString, KMail::Accumulator*> mAccumulators;
  // More info for each folder we care about (mContacts etc. as well as the extra folders)
  // The reason for storing it here is that it can be shared between
  // kmfoldercachedimap and kmfolderimap, and that it's groupware data anyway.
  struct FolderInfo {
    FolderInfo() {} // for QMap
    FolderInfo(KMail::StorageFormat f, KMail::FolderChanges c ) :
      mStorageFormat( f ), mChanges( c ) {}
    KMail::StorageFormat mStorageFormat;
    KMail::FolderChanges mChanges;
  };
  // The storage format used for each folder that we care about
  typedef QMap<KMFolder*, FolderInfo> FolderInfoMap;
  // helper for reading the FolderInfo from the config file
  FolderInfo readFolderInfo( const KMFolder * const folder ) const;

  FolderInfoMap mFolderInfoMap;

  unsigned int mFolderLanguage;

  KMFolderDir* mFolderParentDir;
  KMFolder*    mFolderParent;
  KMFolderType mFolderType;

  bool mUseResourceIMAP;
  bool mResourceQuiet;
  bool mHideFolders;

  /*
   * Bunch of maps to keep track of incidents currently in transfer, ones
   * which need to be ungotten, once we are done, once with updates pending.
   * Since these are transient attributes of only a small but changing number
   * of incidences they are not encapsulated in a struct or somesuch.
   */
  QMap<QString, quint32> mUIDToSerNum;
  QMap<quint32, bool> mTheUnGetMes;
  QMap<QString, QString> mPendingUpdates;
  QMap<QString, bool> mInTransit;
  static QMap<QString, QString> *mSubResourceUINamesMap;
};

#endif // KMAILICALIFACEIMPL_H
