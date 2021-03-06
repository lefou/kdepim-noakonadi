/* ldapsearchdialogimpl.cpp - LDAP access
 *      Copyright (C) 2002 Klar�vdalens Datakonsult AB
 *
 *      Author: Steffen Hansen <hansen@kde.org>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ldapsearchdialog.h"
#include "ldapclient.h"

#include <kabc/addresslineedit.h>
#include <kpimutils/email.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

#include <qcheckbox.h>
#include <qevent.h>
#include <qgroupbox.h>
#include <QHeaderView>
#include <qlabel.h>
#include <qlayout.h>
#include <QTreeWidget>
#include <qlistview.h>
#include <qpushbutton.h>

using namespace KPIM;

static QString asUtf8( const QByteArray &val )
{
  if ( val.isEmpty() ) {
    return QString();
  }

  const char *data = val.data();

  //QString::fromUtf8() bug workaround
  if ( data[ val.size() - 1 ] == '\0' ) {
    return QString::fromUtf8( data, val.size() - 1 );
  } else {
    return QString::fromUtf8( data, val.size() );
  }
}

static QString join( const KLDAP::LdapAttrValue &lst, const QString &sep )
{
  QString res;
  bool alredy = false;
  for ( KLDAP::LdapAttrValue::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( alredy ) {
      res += sep;
    }
    alredy = true;
    res += asUtf8( *it );
  }
  return res;
}

static QMap<QString, QString> &adrbookattr2ldap()
{
  static QMap<QString, QString> keys;

  if ( keys.isEmpty() ) {
    keys[ i18n( "Title" ) ] = "title";
    keys[ i18n( "Full Name" ) ] = "cn";
    keys[ i18n( "Email" ) ] = "mail";
    keys[ i18n( "Home Number" ) ] = "homePhone";
    keys[ i18n( "Work Number" ) ] = "telephoneNumber";
    keys[ i18n( "Mobile Number" ) ] = "mobile";
    keys[ i18n( "Fax Number" ) ] = "facsimileTelephoneNumber";
    keys[ i18n( "Pager" ) ] = "pager";
    keys[ i18n( "Street") ] = "street";
    keys[ i18n( "State" ) ] = "st";
    keys[ i18n( "Country" ) ] = "co";
    keys[ i18n( "City" ) ] = "l";
    keys[ i18n( "Organization" ) ] = "o";
    keys[ i18n( "Company" ) ] = "Company";
    keys[ i18n( "Department" ) ] = "department";
    keys[ i18n( "Zip Code" ) ] = "postalCode";
    keys[ i18n( "Postal Address" ) ] = "postalAddress";
    keys[ i18n( "Description" ) ] = "description";
    keys[ i18n( "User ID" ) ] = "uid";
  }
  return keys;
}

namespace KPIM {

class ContactListItem : public QTreeWidgetItem
{
  public:
    ContactListItem( QTreeWidget *parent, const KLDAP::LdapAttrMap &attrs )
      : QTreeWidgetItem( parent ), mAttrs( attrs )
    {
      const KLDAP::LdapAttrValue &mailAttrs = attrs[ "mail" ];
      if ( mailAttrs.isEmpty() ) {
        setFlags( flags() & ~Qt::ItemIsSelectable );
        setFlags( flags() & ~Qt::ItemIsEnabled );
      }
    }

    KLDAP::LdapAttrMap mAttrs;


};

}

LdapSearchDialog::LdapSearchDialog( QWidget *parent, const char *name )
  : KDialog( parent )
{
  setObjectName( name );
  setCaption( i18n( "Search for Addresses in Directory" ) );
  setButtons( Help | User1 | User2 | User3 | Cancel );
  setDefaultButton( Default );
  showButtonSeparator( true );

  setButtonGuiItem( Cancel, KStandardGuiItem::close() );
  QFrame *page = new QFrame( this );
  setMainWidget( page );
  QVBoxLayout *topLayout = new QVBoxLayout();
  topLayout->setContentsMargins( marginHint(), marginHint(),
                                 marginHint(), marginHint() );
  topLayout->setSpacing( spacingHint() );
  page->setLayout( topLayout );

  QGroupBox *groupBox = new QGroupBox( i18n( "Search for Addresses in Directory" ), page );
  QGridLayout *boxLayout = new QGridLayout();
  boxLayout->setSpacing( spacingHint() );
  groupBox->setLayout( boxLayout );
  boxLayout->setColumnStretch( 1, 1 );

  QLabel *label = new QLabel( i18n( "Search for:" ), groupBox );
  boxLayout->addWidget( label, 0, 0 );

  mSearchEdit = new KLineEdit( groupBox );
  boxLayout->addWidget( mSearchEdit, 0, 1 );
  label->setBuddy( mSearchEdit );

  label = new QLabel( i18n( "in" ), groupBox );
  boxLayout->addWidget( label, 0, 2 );

  mFilterCombo = new KComboBox( groupBox );
  mFilterCombo->addItem( i18n( "Name" ) );
  mFilterCombo->addItem( i18n( "Email" ) );
  mFilterCombo->addItem( i18n( "Home Number" ) );
  mFilterCombo->addItem( i18n( "Work Number" ) );
  boxLayout->addWidget( mFilterCombo, 0, 3 );

  QSize buttonSize;
  mSearchButton = new QPushButton( i18n( "Stop" ), groupBox );
  buttonSize = mSearchButton->sizeHint();
  mSearchButton->setText( i18n( "Search" ) );
  if ( buttonSize.width() < mSearchButton->sizeHint().width() ) {
    buttonSize = mSearchButton->sizeHint();
  }
  mSearchButton->setFixedWidth( buttonSize.width() );

  mSearchButton->setDefault( true );
  boxLayout->addWidget( mSearchButton, 0, 4 );

  mRecursiveCheckbox = new QCheckBox( i18n( "Recursive search" ), groupBox );
  mRecursiveCheckbox->setChecked( true );
  boxLayout->addWidget( mRecursiveCheckbox, 1, 0, 1, 4 );

  mSearchType = new KComboBox( groupBox );
  mSearchType->addItem( i18n( "Contains" ) );
  mSearchType->addItem( i18n( "Starts With" ) );
  boxLayout->addWidget( mSearchType, 1, 3, 1, 2 );

  topLayout->addWidget( groupBox );

  mResultListView = new QTreeWidget( page );
  mResultListView->setAlternatingRowColors( true );
  mResultListView->setSelectionMode( QAbstractItemView::MultiSelection );
  mResultListView->setAllColumnsShowFocus( true );
  mResultListView->setSortingEnabled( true );
  topLayout->addWidget( mResultListView );

  resize( QSize( 600, 400 ).expandedTo( minimumSizeHint() ) );

  setButtonText( User1, i18n( "Unselect All" ) );
  setButtonText( User2, i18n( "Select All" ) );
  setButtonText( User3, i18n( "Add Selected" ) );

  mNumHosts = 0;
  mIsOK = false;

  connect( mRecursiveCheckbox, SIGNAL(toggled(bool)),
           this, SLOT(slotSetScope(bool)) );
  connect( mSearchButton, SIGNAL(clicked()),
           this, SLOT(slotStartSearch()) );

  setTabOrder( mSearchEdit, mFilterCombo );
  setTabOrder( mFilterCombo, mSearchButton );
  mSearchEdit->setFocus();

  restoreSettings();
}

LdapSearchDialog::~LdapSearchDialog()
{
  saveSettings();
}

void LdapSearchDialog::restoreSettings()
{
  // Create one KPIM::LdapClient per selected server and configure it.

  // First clean the list to make sure it is empty at
  // the beginning of the process
  qDeleteAll( mLdapClientList );
  mLdapClientList.clear();

  KConfig kabConfig( "kaddressbookrc" );
  mSearchType->setCurrentIndex( kabConfig.group( "LDAPSearch" ).readEntry( "SearchType", 0 ) );

  // then read the config file and register all selected
  // server in the list
  KConfig config( "kabldaprc" );
  KConfigGroup saver = config.group( "LDAP" );
  mNumHosts = saver.readEntry( "NumSelectedHosts", 0 );
  if ( !mNumHosts ) {
    KMessageBox::error( this,
                        i18n( "You must select a LDAP server before searching.\n"
                              "You can do this from the menu Settings/Configure KAddressBook." ) );
    mIsOK = false;
  } else {
    mIsOK = true;
    for ( int j = 0; j < mNumHosts; ++j ) {
      KLDAP::LdapServer ldapServer;

      QString host = saver.readEntry( QString( "SelectedHost%1" ).arg( j ), "" );
      if ( !host.isEmpty() ) {
        ldapServer.setHost( host );
      }

      int port = saver.readEntry( QString( "SelectedPort%1" ).arg( j ), 0 );
      if ( port ) {
        ldapServer.setPort( port );
      }

      QString base = saver.readEntry( QString( "SelectedBase%1" ).arg( j ), "" );
      if ( !base.isEmpty() ) {
        ldapServer.setBaseDn( KLDAP::LdapDN( base ) );
      }

      QString bindDN = saver.readEntry( QString( "SelectedBind%1" ).arg( j ), "" );
      if ( !bindDN.isEmpty() ) {
        ldapServer.setBindDn( bindDN );
      }

      QString pwdBindDN = saver.readEntry( QString( "SelectedPwdBind%1" ).arg( j ), "" );
      if ( !pwdBindDN.isEmpty() ) {
        ldapServer.setPassword( pwdBindDN );
      }

      KPIM::LdapClient *ldapClient = new KPIM::LdapClient( 0, this, "ldapclient" );
      ldapClient->setServer( ldapServer );

      QStringList attrs;

      for ( QMap<QString,QString>::Iterator it=adrbookattr2ldap().begin();
            it != adrbookattr2ldap().end(); ++it ) {
        attrs << *it;
      }

      ldapClient->setAttrs( attrs );

      connect( ldapClient, SIGNAL(result(const KPIM::LdapClient &,const KLDAP::LdapObject &)),
               this, SLOT(slotAddResult(const KPIM::LdapClient &,const KLDAP::LdapObject &)) );
      connect( ldapClient, SIGNAL(done()),
               this, SLOT(slotSearchDone()) );
      connect( ldapClient, SIGNAL(error(const QString &) ),
               this, SLOT(slotError(const QString &)) );

      mLdapClientList.append( ldapClient );
    }

    // Remove existing columns
    mResultListView->setColumnCount( 0 );

    QStringList headerLabels;
    headerLabels << i18n( "Full Name" )
                 << i18n( "Email" )
                 << i18n( "Home Number" )
                 << i18n( "Work Number" )
                 << i18n( "Mobile Number" )
                 << i18n( "Fax Number" )
                 << i18n( "Company" )
                 << i18n( "Organization" )
                 << i18n( "Street" )
                 << i18n( "State" )
                 << i18n( "Country" )
                 << i18n( "Zip Code" )
                 << i18n( "Postal Address" )
                 << i18n( "City" )
                 << i18n( "Department" )
                 << i18n( "Description" )
                 << i18n( "User ID" )
                 << i18n( "Title" );

    mResultListView->setHeaderLabels( headerLabels );
    mResultListView->setColumnWidth( 0, 200 );
    mResultListView->setColumnWidth( 1, 200 );
    mResultListView->clear();
  }
}

void LdapSearchDialog::saveSettings()
{
  KConfig config( "kaddressbookrc" );
  KConfigGroup group = config.group( "LDAPSearch" );
  group.writeEntry( "SearchType", mSearchType->currentIndex() );
  config.sync();
}

void LdapSearchDialog::cancelQuery()
{
  foreach ( KPIM::LdapClient *client, mLdapClientList ) {
    client->cancelQuery();
  }
}

void LdapSearchDialog::slotAddResult( const KPIM::LdapClient &, const KLDAP::LdapObject &obj )
{
  ContactListItem *itemToAdd = new ContactListItem( mResultListView, obj.attributes() );

  for ( int column = 0; column < mResultListView->columnCount(); column++ )
  {
    const QString colName = mResultListView->model()->headerData( column, Qt::Horizontal ).toString();
    const QString ldapAttrName = adrbookattr2ldap()[ colName ];
    itemToAdd->setText( column, join( itemToAdd->mAttrs[ ldapAttrName ], ", " ) );
  }

  mResultListView->addTopLevelItem( itemToAdd );
}

void LdapSearchDialog::slotSetScope( bool rec )
{
  foreach ( KPIM::LdapClient *client, mLdapClientList ) {
    if ( rec ) {
      client->setScope( "sub" );
    } else {
      client->setScope( "one" );
    }
  }
}

QString LdapSearchDialog::makeFilter( const QString &query, const QString &attr,
                                      bool startsWith )
{
  /* The reasoning behind this filter is:
   * If a person or a distlist=>show it, even if it doesn't have an email address.
   * If not a person or a distlist=>only show it if it has an email attribute.
   * This allows both resource accounts with an email address which are not
   * a person and person entries without an email address to show up, while
   * still not showing things like structural entries in the ldap tree. */
  QString result( "&(|(objectclass=person)(objectclass=groupofnames)(mail=*))(" );
  if( query.isEmpty() ) {
    // Return a filter that matches everything
    return result + "|(cn=*)(sn=*)" + ')';
  }

  if ( attr == i18n( "Name" ) ) {
    result += startsWith ? "|(cn=%1*)(sn=%2*)" : "|(cn=*%1*)(sn=*%2*)";
    result = result.arg( query ).arg( query );
  } else {
    result += ( startsWith ? "%1=%2*" : "%1=*%2*" );
    if ( attr == i18n( "Email" ) ) {
      result = result.arg( "mail" ).arg( query );
    } else if ( attr == i18n( "Home Number" ) ) {
      result = result.arg( "homePhone" ).arg( query );
    } else if ( attr == i18n( "Work Number" ) ) {
      result = result.arg( "telephoneNumber" ).arg( query );
    } else {
      // Error?
      result.clear();
      return result;
    }
  }
  result += ')';
  return result;
}

void LdapSearchDialog::slotStartSearch()
{
  cancelQuery();

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mSearchButton->setText( i18n( "Stop" ) );

  disconnect( mSearchButton, SIGNAL(clicked()),
              this, SLOT(slotStartSearch()) );
  connect( mSearchButton, SIGNAL(clicked()),
           this, SLOT(slotStopSearch()) );

  bool startsWith = ( mSearchType->currentIndex() == 1 );

  QString filter = makeFilter( mSearchEdit->text().trimmed(),
                               mFilterCombo->currentText(), startsWith );

   // loop in the list and run the KPIM::LdapClients
  mResultListView->clear();
  foreach ( KPIM::LdapClient *client, mLdapClientList ) {
    client->startQuery( filter );
  }

  saveSettings();
}

void LdapSearchDialog::slotStopSearch()
{
  cancelQuery();
  slotSearchDone();
}

void LdapSearchDialog::slotSearchDone()
{
  // If there are no more active clients, we are done.
  foreach ( KPIM::LdapClient *client, mLdapClientList ) {
    if ( client->isActive() ) {
      return;
    }
  }

  disconnect( mSearchButton, SIGNAL(clicked()),
              this, SLOT(slotStopSearch()) );
  connect( mSearchButton, SIGNAL(clicked()),
           this, SLOT(slotStartSearch()) );

  mSearchButton->setText( i18n( "Search" ) );
  QApplication::restoreOverrideCursor();
}

void LdapSearchDialog::slotError( const QString &error )
{
  QApplication::restoreOverrideCursor();
  KMessageBox::error( this, error );
}

void LdapSearchDialog::closeEvent( QCloseEvent *e )
{
  slotStopSearch();
  e->accept();
}

/*!
 * Returns a ", " separated list of email addresses that were
 * checked by the user
 */
QString LdapSearchDialog::selectedEMails() const
{
  QStringList result;
  for ( int i = 0; i < mResultListView->topLevelItemCount(); i++ ) {
    ContactListItem *cli = static_cast<ContactListItem *>( mResultListView->topLevelItem( i ) );
    if ( cli->isSelected() ) {
      const KLDAP::LdapAttrValue &mailAttrs = cli->mAttrs[ "mail" ];
      if ( mailAttrs.size() >= 1 ) {
        QString email = asUtf8( mailAttrs.first() ).trimmed();
        if ( !email.isEmpty() ) {
          const KLDAP::LdapAttrValue &nameAttrs = cli->mAttrs[ "cn" ];
          QString name;
          if ( nameAttrs.size() > 1 ) {
            name = asUtf8( nameAttrs.first() ).trimmed();
          }
          if ( name.isEmpty() ) {
            result << email;
          } else {
            result << KPIMUtils::quoteNameIfNecessary( name ) + " <" + email + '>';
          }
        }
      }
    }
  }

  return result.join( ", " );
}

void LdapSearchDialog::slotButtonClicked( int button )
{
  switch ( button ) {
  case Help:
    KToolInvocation::self()->invokeHelp( "ldap-queries" );
    break;
  case User1:
    mResultListView->clearSelection();
    break;
  case User2:
    mResultListView->selectAll();
    break;
  case User3:
    emit addresseesAdded();
    break;
  default:
    KDialog::slotButtonClicked(button);
  }
}

#include "ldapsearchdialog.moc"
