/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kabconfigwidget.h"

#include <QtGui/QBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktabwidget.h>

#include "addresseewidget.h"
#include "kabprefs.h"

KABConfigWidget::KABConfigWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  KTabWidget *tabWidget = new KTabWidget( this );
  topLayout->addWidget( tabWidget );

  // General page
  QWidget *generalPage = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( generalPage );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  QGroupBox *groupBox = new QGroupBox( i18n( "General" ), generalPage );
  QBoxLayout *boxLayout = new QVBoxLayout( groupBox );
  boxLayout->setAlignment( Qt::AlignTop );

  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), groupBox );
  boxLayout->addWidget( mViewsSingleClickBox );

  mNameParsing = new QCheckBox( i18n( "Automatic name parsing for new addressees" ), groupBox );
  boxLayout->addWidget( mNameParsing );

  mLimitContactDisplay = new QCheckBox( i18n( "Limit unfiltered display to 100 contacts" ), groupBox );
  boxLayout->addWidget( mLimitContactDisplay );

  QBoxLayout *editorLayout = new QHBoxLayout();
  boxLayout->addItem( editorLayout );
  editorLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Addressee editor type:" ), groupBox );
  editorLayout->addWidget( label );

  mEditorCombo = new KComboBox( groupBox );
  mEditorCombo->addItem( i18n( "Full Editor" ) );
  mEditorCombo->addItem( i18n( "Simple Editor" ) );
  label->setBuddy( mEditorCombo );
  editorLayout->addWidget( mEditorCombo );

  editorLayout->addStretch( 1 );


  layout->addWidget( groupBox );

  groupBox = new QGroupBox( i18n( "Script-Hooks" ), generalPage );
  QGridLayout *grid = new QGridLayout( groupBox );
  grid->setSpacing( KDialog::spacingHint() );
  label = new QLabel( i18n( "Phone:" ), groupBox );
  grid->addWidget( label, 0, 0 );

  mPhoneHook = new KLineEdit( groupBox );
  mPhoneHook->setToolTip( i18n( "<ul><li>%N: Phone Number</li></ul>" ) );
  grid->addWidget( mPhoneHook, 0, 1 );

  label = new QLabel( i18n( "Fax:" ), groupBox );
  grid->addWidget( label, 1, 0 );

  mFaxHook = new KLineEdit( groupBox );
  mFaxHook->setToolTip( i18n( "<ul><li>%N: Fax Number</li></ul>" ) );
  grid->addWidget( mFaxHook, 1, 1 );


  label = new QLabel( i18n( "SMS text:" ), groupBox );
  grid->addWidget( label, 2, 0 );

  mSMSHook = new KLineEdit( groupBox );
  mSMSHook->setToolTip( i18n( "<ul><li>%N: Phone Number</li><li>%F: File containing the text message(s)</li></ul>" ) );
  grid->addWidget( mSMSHook, 2, 1 );


  grid->setColumnStretch( 1, 1 );

  layout->addWidget( groupBox );

  groupBox = new QGroupBox( i18n( "Location Map" ), generalPage );
  boxLayout = new QVBoxLayout( groupBox );
  boxLayout->setSpacing( KDialog::spacingHint() );
  boxLayout->setAlignment( Qt::AlignTop );

  mLocationMapURL = new KComboBox( groupBox );
  mLocationMapURL->setEditable( true );
  mLocationMapURL->setMinimumContentsLength( 20 );
  mLocationMapURL->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLength );
  mLocationMapURL->setToolTip( i18n( "<ul> <li>%s: Street</li>"
                                 "<li>%r: Region</li>"
                                 "<li>%l: Location</li>"
                                 "<li>%z: Zip Code</li>"
                                 "<li>%c: Country ISO Code</li> </ul>" ) );
  boxLayout->addWidget( mLocationMapURL );
  layout->addWidget( groupBox );

  connect( mNameParsing, SIGNAL( toggled( bool ) ), SLOT( modified() ) );
  connect( mViewsSingleClickBox, SIGNAL( toggled( bool ) ), SLOT( modified() ) );
  connect( mLimitContactDisplay, SIGNAL( toggled( bool ) ), SLOT( modified() ) );
  connect( mPhoneHook, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mSMSHook, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mFaxHook, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mLocationMapURL, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mEditorCombo, SIGNAL( activated( int ) ), SLOT( modified() ) );

  tabWidget->addTab( generalPage, i18n( "General" ) );

  // Addressee page
  mAddresseeWidget = new AddresseeWidget( this );
  tabWidget->addTab( mAddresseeWidget, i18n( "Contact" ) );
  connect( mAddresseeWidget, SIGNAL( modified() ), SLOT( modified() ) );

  mLocationMapURL->addItems( KABPrefs::instance()->locationMapURLs() );
}

void KABConfigWidget::restoreSettings()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mNameParsing->setChecked( KABPrefs::instance()->automaticNameParsing() );
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->honorSingleClick() );
  mPhoneHook->setText( KABPrefs::instance()->phoneHookApplication() );
  mSMSHook->setText( KABPrefs::instance()->sMSHookApplication() );
  mFaxHook->setText( KABPrefs::instance()->faxHookApplication() );
  mAddresseeWidget->restoreSettings();
  mEditorCombo->setCurrentIndex( KABPrefs::instance()->editorType() );
  mLocationMapURL->setItemText( mLocationMapURL->currentIndex(),
      KABPrefs::instance()->locationMapURL().arg( KGlobal::locale()->country() ) );
  mLocationMapURL->lineEdit()->setCursorPosition( 0 );

  KConfig _config("kabcrc", KConfig::NoGlobals);
  KConfigGroup config(&_config, "General" );
  mLimitContactDisplay->setChecked( config.readEntry( "LimitContactDisplay", true ) );

  blockSignals( blocked );

  emit changed( false );
}

void KABConfigWidget::saveSettings()
{
  KABPrefs::instance()->setAutomaticNameParsing( mNameParsing->isChecked() );
  KABPrefs::instance()->setHonorSingleClick( mViewsSingleClickBox->isChecked() );
  KABPrefs::instance()->setPhoneHookApplication( mPhoneHook->text() );
  KABPrefs::instance()->setSMSHookApplication( mSMSHook->text() );
  KABPrefs::instance()->setFaxHookApplication( mFaxHook->text() );
  KABPrefs::instance()->setEditorType( mEditorCombo->currentIndex() );
  KABPrefs::instance()->setLocationMapURL( mLocationMapURL->currentText() );
  mAddresseeWidget->saveSettings();

  KABPrefs::instance()->writeConfig();

  KConfig _config("kabcrc", KConfig::NoGlobals);
  KConfigGroup config(&_config, "General" );
  config.writeEntry( "LimitContactDisplay", mLimitContactDisplay->isChecked() );

  emit changed( false );
}

void KABConfigWidget::defaults()
{
  mNameParsing->setChecked( true );
  mViewsSingleClickBox->setChecked( false );
  mEditorCombo->setCurrentIndex( 0 );
  mLimitContactDisplay->setChecked( true );

  emit changed( true );
}

void KABConfigWidget::modified()
{
  emit changed( true );
}

#include "kabconfigwidget.moc"
