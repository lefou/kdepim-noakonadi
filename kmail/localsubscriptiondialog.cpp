/*  -*- c++ -*-
  localsubscriptiondialog.cpp

  This file is part of KMail, the KDE mail client.
  Copyright (C) 2006 Till Adam <adam@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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

#include "localsubscriptiondialog.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kmkernel.h"
#include "accountmanager.h"
#include "kmmessage.h"
#include "imapaccountbase.h"
#include "listjob.h"

namespace KMail {

LocalSubscriptionDialog::LocalSubscriptionDialog( QWidget *parent, const QString &caption,
    ImapAccountBase *acct, const QString &startPath )
  : SubscriptionDialog( parent, caption, acct, startPath ),
    mAccount( acct )
{
}

/* virtual */
LocalSubscriptionDialog::~LocalSubscriptionDialog()
{

}

void LocalSubscriptionDialog::listAllAvailableAndCreateItems()
{
  if ( mAccount->onlySubscribedFolders() )
    mSubscribed = true;
  SubscriptionDialog::listAllAvailableAndCreateItems();
}

/* virtual */
void LocalSubscriptionDialog::processFolderListing()
{
  uint done = 0;
  for (int i = mCount; i < mFolderNames.count(); ++i)
  {
    // give the dialog a chance to repaint
    if (done == 1000)
    {
      emit listChanged();
      QTimer::singleShot(0, this, SLOT(processItems()));
      return;
    }
    ++mCount;
    ++done;
    createListViewItem( i );
  }

  if ( mPrefixList.isEmpty() && !mSubscribed )
    loadingComplete(); // no need to load subscribed folders
  else
    processNext();
}

void LocalSubscriptionDialog::setCheckedStateOfAllItems()
{
   // iterate over all items and check them, unless they are
   // in the account's local subscription blacklist
  QMapIterator<QString, GroupItem*> it( mItemDict );
  while ( it.hasNext() ) {
    it.next();
    QString path = it.key();
    GroupItem *item = it.value();
    item->setOn( mAccount->locallySubscribedTo( path ) );
  }
}

/*virtual*/
bool LocalSubscriptionDialog::doSave()
{
  if ( !checkIfSubscriptionsEnabled() )
    return false;

  bool somethingHappened = false;
  // subscribe
  QTreeWidgetItemIterator it(subView);
  for ( ; *it; ++it) {
    static_cast<ImapAccountBase*>(account())->changeLocalSubscription(
        static_cast<GroupItem*>(*it)->info().path, true );
    somethingHappened = true;
  }

  // unsubscribe
  QTreeWidgetItemIterator it2(unsubView);
  if ( unsubView->topLevelItemCount() > 0 ) {
    const QString message = i18nc( "@info", "Locally unsubscribing from folders will remove all "
        "information that is present locally about those folders. The folders will "
        "not be changed on the server. Press cancel now if you want to make sure "
        "all local changes have been written to the server by checking mail first.");
    const QString caption = i18nc( "@title:window", "Local changes will be lost when "
                                  "unsubscribing" );
    if ( KMessageBox::warningContinueCancel( this, message, caption )
        != KMessageBox::Cancel ) {
      somethingHappened = true;
      for ( ; *it2; ++it2) {
        static_cast<ImapAccountBase*>(account())->changeLocalSubscription(
            static_cast<GroupItem*>(*it2)->info().path, false );
      }

    }
  }

  // Slight code duplication with SubscriptionDialog follows!
  KMail::ImapAccountBase *a = static_cast<KMail::ImapAccountBase*>( mAcct );
  if ( mForceSubscriptionEnable ) {
    a->setOnlyLocallySubscribedFolders( true );
  }

  if ( somethingHappened && subscriptionOptionEnabled( a ) ) {
    kmkernel->acctMgr()->singleCheckMail( mAccount, true);
  }

  return true;
}

bool LocalSubscriptionDialog::subscriptionOptionEnabled( const KMail::ImapAccountBase *account ) const
{
  return account->onlyLocallySubscribedFolders();
}

QString LocalSubscriptionDialog::subscriptionOptionQuestion( const QString &accountName ) const
{
  return i18nc( "@info", "Currently local subscriptions are not used for account <resource>%1</resource>.<nl/>"
      "\nDo you want to enable local subscriptions?", accountName );
}

void LocalSubscriptionDialog::loadingComplete()
{
  setCheckedStateOfAllItems();
  SubscriptionDialog::loadingComplete();
}

} // namespace

#include "localsubscriptiondialog.moc"
