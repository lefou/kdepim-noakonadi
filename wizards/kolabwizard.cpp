/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Daniel Molkentin <molkentin@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kolabwizard.h"
#include "kolabconfig.h"

#include "kmailchanges.h"

#include <libkcal/resourcecalendar.h>
#include <kabc/resource.h>

#include "kresources/kolab/kcal/resourcekolab.h"
#include "kresources/kolab/kabc/resourcekolab.h"
#include "kresources/kolab/knotes/resourcekolab.h"

#include <qwhatsthis.h>
#include <klineedit.h>
#include <klocale.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

class SetupLDAPSearchAccount : public KConfigPropagator::Change
{
  public:
    SetupLDAPSearchAccount()
      : KConfigPropagator::Change( i18n("Setup LDAP Search Account") )
    {
    }

    void apply()
    {
      QString host = KolabConfig::self()->server();
      QString basedn = host;
      basedn.replace(".",",dc=");
      basedn.prepend("dc=");

      KConfig c( "kabldaprc" );
      c.setGroup( "LDAP" );
      bool hasMyServer = false;
      uint selHosts = c.readNumEntry("NumSelectedHosts", 0);
      for ( uint i = 0 ; i < selHosts && !hasMyServer; ++i )
        if ( c.readEntry( QString("SelectedHost%1").arg(i) ) == host )
          hasMyServer = true;
      if ( !hasMyServer ) {
        c.writeEntry( "NumSelectedHosts", selHosts + 1 );
        c.writeEntry( QString("SelectedHost%1").arg(selHosts), host);
        c.writeEntry( QString("SelectedBase%1").arg(selHosts), basedn);
        c.writeEntry( QString("SelectedPort%1").arg(selHosts), "389");
      }
    }

};

class CreateCalendarKolabResource : public KConfigPropagator::Change
{
  public:
    CreateCalendarKolabResource()
      : KConfigPropagator::Change( i18n("Create Calendar Kolab Resource") )
    {
    }

    void apply()
    {
      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();
      //QString server = KolabConfig::self()->server();
      KCal::ResourceKolab *r = new KCal::ResourceKolab( 0 );
      r->setResourceName( i18n("Kolab Server") );
      r->setName( "kolab-resource" );
      m.add( r );
      m.writeConfig();
    }
};

class CreateContactKolabResource : public KConfigPropagator::Change
{
  public:
    CreateContactKolabResource()
      : KConfigPropagator::Change( i18n("Create Contact Kolab Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<KABC::Resource> m( "contact" );
      m.readConfig();
      KABC::ResourceKolab *r = new KABC::ResourceKolab( 0 );
      r->setResourceName( i18n("Kolab Server") );
      r->setName( "kolab-resource" );
      m.add( r );
      m.writeConfig();
    }

};

class CreateNotesKolabResource : public KConfigPropagator::Change
{
  public:
    CreateNotesKolabResource()
      : KConfigPropagator::Change( i18n("Create Notes Kolab Resource") )
    {
    }

    void apply()
    {
      KRES::Manager<ResourceNotes> m( "notes" );
      m.readConfig();
      Kolab::ResourceKolab *r = new Kolab::ResourceKolab( 0 );
      r->setResourceName( i18n("Kolab Server") );
      r->setName( "kolab-resource" );
      m.add( r );
      m.writeConfig();
    }

};


class KolabPropagator : public KConfigPropagator
{
  public:
    KolabPropagator()
      : KConfigPropagator( KolabConfig::self(), "kolab.kcfg" )
    {
    }

  protected:
    void addKorganizerChanges( Change::List &changes )
    {
      KURL freeBusyBaseUrl = "webdavs://" + KolabConfig::self()->server() +
                                "/freebusy/";

      ChangeConfig *c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";

      c->name = "FreeBusyPublishUrl";
      c->label = "";

      QString user = KolabConfig::self()->user();
      // We now use the full email address in the freebusy URL
      //int pos = user.find( "@" );
      //if ( pos > 0 ) user = user.left( pos );

      KURL publishURL = freeBusyBaseUrl;
      publishURL.addPath( user + ".ifb" ); // this encodes the '@' in the username
      c->value = publishURL.url();

      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyRetrieveUrl";
      c->value = freeBusyBaseUrl.url();
      changes.append( c );

      // Use full email address for retrieval of free/busy lists
      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "FreeBusy";
      c->name = "FreeBusyFullDomainRetrieval";
      c->value = "true";
      changes.append( c );

      c = new ChangeConfig;
      c->file = "korganizerrc";
      c->group = "Group Scheduling";
      c->name = "Use Groupware Communication";
      c->value = "true";
      changes.append( c );
    }

    virtual void addCustomChanges( Change::List &changes )
    {
      addKorganizerChanges( changes );

      // KMail cruft has been outsourced to kmailchanges.cpp
      createKMailChanges( changes );

      changes.append( new SetupLDAPSearchAccount );

      KCal::CalendarResourceManager m( "calendar" );
      m.readConfig();
      KCal::CalendarResourceManager::Iterator it;
      for ( it = m.begin(); it != m.end(); ++it ) {
        if ( (*it)->type() == "kolab" ) break;
      }
      if ( it == m.end() ) {
        changes.append( new CreateCalendarKolabResource );
        changes.append( new CreateContactKolabResource );
        changes.append( new CreateNotesKolabResource );
      }
    }
};

KolabWizard::KolabWizard() : KConfigWizard( new KolabPropagator )
{
  QFrame *page = createWizardPage( i18n("Kolab Server") );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("Server name:"), page );
  topLayout->addWidget( label, 0, 0 );
  mServerEdit = new KLineEdit( page );
  topLayout->addWidget( mServerEdit, 0, 1 );

  label = new QLabel( i18n("Kolab user name:"), page );
  topLayout->addWidget( label, 1, 0 );
  mUserEdit = new KLineEdit( page );
  topLayout->addWidget( mUserEdit, 1, 1 );
  QWhatsThis::add(mUserEdit, i18n("Your Kolab Server user ID. "
                        "Format: <i>name@server.domain.tld</i>"));

  label = new QLabel( i18n("Real name:"), page );
  topLayout->addWidget( label, 2, 0 );
  mRealNameEdit = new KLineEdit( page );
  topLayout->addWidget( mRealNameEdit, 2, 1 );

  label = new QLabel( i18n("Password:"), page );
  topLayout->addWidget( label, 3, 0 );
  mPasswordEdit = new KLineEdit( page );
  mPasswordEdit->setEchoMode( KLineEdit::Password );
  topLayout->addWidget( mPasswordEdit, 3, 1 );

  mSavePasswordCheck = new QCheckBox( i18n("Save password"), page );
  topLayout->addMultiCellWidget( mSavePasswordCheck, 4, 4, 0, 1 );

  topLayout->setRowStretch( 4, 1 );

  //DF: I don't see the point in showing the user those pages.
  //They are very 'internal' and of no use to anyone other than developers.
  //(This is even more true for the rules page. The changes page is sort of OK)

  //setupRulesPage();
  setupChangesPage();

  setInitialSize( QSize( 600, 300 ) );
}

KolabWizard::~KolabWizard()
{
}

void KolabWizard::usrReadConfig()
{
  mServerEdit->setText( KolabConfig::self()->server() );
  mUserEdit->setText( KolabConfig::self()->user() );
  mRealNameEdit->setText( KolabConfig::self()->realName() );
  mPasswordEdit->setText( KolabConfig::self()->password() );
  mSavePasswordCheck->setChecked( KolabConfig::self()->savePassword() );
}

void KolabWizard::usrWriteConfig()
{
  KolabConfig::self()->setServer( mServerEdit->text() );
  KolabConfig::self()->setUser( mUserEdit->text() );
  KolabConfig::self()->setRealName( mRealNameEdit->text() );
  KolabConfig::self()->setPassword( mPasswordEdit->text() );
  KolabConfig::self()->setSavePassword( mSavePasswordCheck->isChecked() );
}
