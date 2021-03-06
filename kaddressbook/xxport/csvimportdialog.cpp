/*
   This file is part of KAddressBook.
   Copyright (C) 2003 Tobias Koenig <tokoe@kde.org>
                 based on the code of KSpread's CSV Import Dialog

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
#include "csvimportdialog.h"

#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QScrollBar>
#include <QtGui/QTableWidget>

#include <kabc/addressbook.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "comboboxheaderview.h"
#include "dateparser.h"

enum { Local = 0, Guess = 1, Latin1 = 2, Uni = 3, MSBug = 4, Codec = 5 };

CSVImportDialog::CSVImportDialog( KABC::AddressBook *ab, QWidget *parent )
  : KDialog( parent ),
    mAdjustRows( false ),
    mStartLine( 0 ),
    mTextQuote( '"' ),
    mDelimiter( "," ),
    mAddressBook( ab )
{
  setCaption( i18nc( "@title:window", "CSV Import Dialog" ) );
  setButtons( Ok | Cancel | User1 | User2 );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  mTypeMap.insert( i18nc( "@item Undefined import field type", "Undefined" ), Undefined );
  mTypeMap.insert( KABC::Addressee::formattedNameLabel(), FormattedName );
  mTypeMap.insert( KABC::Addressee::familyNameLabel(), FamilyName );
  mTypeMap.insert( KABC::Addressee::givenNameLabel(), GivenName );
  mTypeMap.insert( KABC::Addressee::additionalNameLabel(), AdditionalName );
  mTypeMap.insert( KABC::Addressee::prefixLabel(), Prefix );
  mTypeMap.insert( KABC::Addressee::suffixLabel(), Suffix );
  mTypeMap.insert( KABC::Addressee::nickNameLabel(), NickName );
  mTypeMap.insert( KABC::Addressee::birthdayLabel(), Birthday );

  mTypeMap.insert( KABC::Addressee::homeAddressStreetLabel(), HomeAddressStreet );
  mTypeMap.insert( KABC::Addressee::homeAddressLocalityLabel(),
                   HomeAddressLocality );
  mTypeMap.insert( KABC::Addressee::homeAddressRegionLabel(), HomeAddressRegion );
  mTypeMap.insert( KABC::Addressee::homeAddressPostalCodeLabel(),
                   HomeAddressPostalCode );
  mTypeMap.insert( KABC::Addressee::homeAddressCountryLabel(),
                   HomeAddressCountry );
  mTypeMap.insert( KABC::Addressee::homeAddressLabelLabel(), HomeAddressLabel );

  mTypeMap.insert( KABC::Addressee::businessAddressStreetLabel(),
                   BusinessAddressStreet );
  mTypeMap.insert( KABC::Addressee::businessAddressLocalityLabel(),
                   BusinessAddressLocality );
  mTypeMap.insert( KABC::Addressee::businessAddressRegionLabel(),
                   BusinessAddressRegion );
  mTypeMap.insert( KABC::Addressee::businessAddressPostalCodeLabel(),
                   BusinessAddressPostalCode );
  mTypeMap.insert( KABC::Addressee::businessAddressCountryLabel(),
                   BusinessAddressCountry );
  mTypeMap.insert( KABC::Addressee::businessAddressLabelLabel(),
                   BusinessAddressLabel );

  mTypeMap.insert( KABC::Addressee::homePhoneLabel(), HomePhone );
  mTypeMap.insert( KABC::Addressee::businessPhoneLabel(), BusinessPhone );
  mTypeMap.insert( KABC::Addressee::mobilePhoneLabel(), MobilePhone );
  mTypeMap.insert( KABC::Addressee::homeFaxLabel(), HomeFax );
  mTypeMap.insert( KABC::Addressee::businessFaxLabel(), BusinessFax );
  mTypeMap.insert( KABC::Addressee::carPhoneLabel(), CarPhone );
  mTypeMap.insert( KABC::Addressee::isdnLabel(), Isdn );
  mTypeMap.insert( KABC::Addressee::pagerLabel(), Pager );
  mTypeMap.insert( KABC::Addressee::emailLabel(), Email );
  mTypeMap.insert( KABC::Addressee::mailerLabel(), Mailer );
  mTypeMap.insert( KABC::Addressee::titleLabel(), Title );
  mTypeMap.insert( KABC::Addressee::roleLabel(), Role );
  mTypeMap.insert( KABC::Addressee::organizationLabel(), Organization );
  mTypeMap.insert( KABC::Addressee::noteLabel(), Note );
  mTypeMap.insert( KABC::Addressee::urlLabel(), URL );

  initGUI();

  mCustomCounter = mTypeMap.count();
  int count = mCustomCounter;

  KABC::Field::List fields = mAddressBook->fields( KABC::Field::CustomCategory );
  KABC::Field::List::Iterator it;
  for ( it = fields.begin(); it != fields.end(); ++it, ++count )
    mTypeMap.insert( (*it)->label(), count );

  reloadCodecs();

  connect( mDelimiterGroup, SIGNAL( buttonClicked( int ) ),
           this, SLOT( delimiterClicked( int ) ) );
  connect( mDelimiterEdit, SIGNAL( returnPressed() ),
           this, SLOT( returnPressed() ) );
  connect( mDelimiterEdit, SIGNAL( textChanged ( const QString& ) ),
           this, SLOT( textChanged ( const QString& ) ) );
  connect( mComboLine, SIGNAL( activated( const QString& ) ),
           this, SLOT( lineSelected( const QString& ) ) );
  connect( mComboQuote, SIGNAL( activated( const QString& ) ),
           this, SLOT( textquoteSelected( const QString& ) ) );
  connect( mIgnoreDuplicates, SIGNAL( stateChanged( int ) ),
           this, SLOT( ignoreDuplicatesChanged( int ) ) );
  connect( mCodecCombo, SIGNAL( activated( const QString& ) ),
           this, SLOT( codecChanged() ) );

  connect( mUrlRequester, SIGNAL( returnPressed( const QString& ) ),
           this, SLOT( setFile( const QString& ) ) );
  connect( mUrlRequester, SIGNAL( urlSelected( const KUrl& ) ),
           this, SLOT( setFile( const KUrl& ) ) );
  connect( mUrlRequester->lineEdit(), SIGNAL( textChanged ( const QString& ) ),
           this, SLOT( urlChanged( const QString& ) ) );

  connect( this, SIGNAL( user1Clicked() ),
           this, SLOT( applyTemplate() ) );

  connect( this, SIGNAL( user2Clicked() ),
           this, SLOT( saveTemplate() ) );
  connect( this, SIGNAL( okClicked()),
           this, SLOT( slotOk()));
}

CSVImportDialog::~CSVImportDialog()
{
  mCodecs.clear();
}

KABC::AddresseeList CSVImportDialog::contacts() const
{
  DateParser dateParser( mDatePatternEdit->text() );
  KABC::AddresseeList contacts;

  KProgressDialog progressDialog( mPage );
  progressDialog.setAutoClose( true );
  progressDialog.progressBar()->setMaximum( mTable->rowCount() );
  progressDialog.setLabelText( i18nc( "@label", "Importing contacts" ) );
  progressDialog.show();

  kapp->processEvents();

  ComboBoxHeaderView *headerView = static_cast< ComboBoxHeaderView * >
                                     ( mTable->horizontalHeader() );

  for ( int row = 0; row < mTable->rowCount(); ++row ) {
    KABC::Addressee a;
    bool emptyRow = true;
    KABC::Address addrHome( KABC::Address::Home );
    KABC::Address addrWork( KABC::Address::Work );
    for ( int col = 0; col < mTable->columnCount(); ++col ) {
      QString value;
      if ( mTable->item( row, col ) ) {
        value = mTable->item( row, col )->text();
      }

      if ( !value.isEmpty() )
        emptyRow = false;

      switch ( mTypeMap.value( mTable->model()->headerData( col, Qt::Horizontal ).toString() ) ) {
        case Undefined:
          continue;
          break;
        case FormattedName:
          a.setFormattedName( value );
          break;
        case GivenName:
          a.setGivenName( value );
          break;
        case FamilyName:
          a.setFamilyName( value );
          break;
        case AdditionalName:
          a.setAdditionalName( value );
          break;
        case Prefix:
          a.setPrefix( value );
          break;
        case Suffix:
          a.setSuffix( value );
          break;
        case NickName:
          a.setNickName( value );
          break;
        case Birthday:
          a.setBirthday( QDateTime( dateParser.parse( value ) ) );
          break;
        case Email:
          if ( !value.isEmpty() )
            a.insertEmail( value, true );
          break;
        case Role:
          a.setRole( value );
          break;
        case Title:
          a.setTitle( value );
          break;
        case Mailer:
          a.setMailer( value );
          break;
        case URL:
          a.setUrl( KUrl( value ) );
          break;
        case Organization:
          a.setOrganization( value );
          break;
        case Note:
          a.setNote( a.note() + value + '\n' );
          break;

        case HomePhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Home );
            a.insertPhoneNumber( number );
          }
          break;
        case BusinessPhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Work );
            a.insertPhoneNumber( number );
          }
          break;
        case MobilePhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Cell );
            a.insertPhoneNumber( number );
          }
          break;
        case HomeFax:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Home |
                                             KABC::PhoneNumber::Fax );
            a.insertPhoneNumber( number );
          }
          break;
        case BusinessFax:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Work |
                                             KABC::PhoneNumber::Fax );
            a.insertPhoneNumber( number );
          }
          break;
        case CarPhone:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Car );
            a.insertPhoneNumber( number );
          }
          break;
        case Isdn:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Isdn );
            a.insertPhoneNumber( number );
          }
          break;
        case Pager:
          if ( !value.isEmpty() ) {
            KABC::PhoneNumber number( value, KABC::PhoneNumber::Pager );
            a.insertPhoneNumber( number );
          }
          break;

        case HomeAddressStreet:
          addrHome.setStreet( value );
          break;
        case HomeAddressLocality:
          addrHome.setLocality( value );
          break;
        case HomeAddressRegion:
          addrHome.setRegion( value );
          break;
        case HomeAddressPostalCode:
          addrHome.setPostalCode( value );
          break;
        case HomeAddressCountry:
          addrHome.setCountry( value );
          break;
        case HomeAddressLabel:
          addrHome.setLabel( value );
          break;

        case BusinessAddressStreet:
          addrWork.setStreet( value );
          break;
        case BusinessAddressLocality:
          addrWork.setLocality( value );
          break;
        case BusinessAddressRegion:
          addrWork.setRegion( value );
          break;
        case BusinessAddressPostalCode:
          addrWork.setPostalCode( value );
          break;
        case BusinessAddressCountry:
          addrWork.setCountry( value );
          break;
        case BusinessAddressLabel:
          addrWork.setLabel( value );
          break;
        default:
          KABC::Field::List fields = mAddressBook->fields( KABC::Field::CustomCategory );
          KABC::Field::List::Iterator it;

          int counter = 0;
          for ( it = fields.begin(); it != fields.end(); ++it ) {
            if ( counter == (int)( mTypeMap.value( mTable->model()->headerData( col, Qt::Horizontal ).toString() ) - mCustomCounter ) ) {
              (*it)->setValue( a, value );
              break;
            }
            ++counter;
          }
          break;
      }
    }

    kapp->processEvents();

    if ( progressDialog.wasCancelled() )
      return KABC::AddresseeList();

    progressDialog.progressBar()->setValue( progressDialog.progressBar()->value() + 1 );

    if ( !addrHome.isEmpty() )
      a.insertAddress( addrHome );
    if ( !addrWork.isEmpty() )
      a.insertAddress( addrWork );

    if ( !emptyRow && !a.isEmpty() )
      contacts.append( a );
  }

  return contacts;
}

void CSVImportDialog::initGUI()
{
  mPage = new QWidget( this );
  setMainWidget( mPage );

  QGridLayout *layout = new QGridLayout( mPage );
  layout->setSpacing( spacingHint() );
  layout->setMargin( 0 );
  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18nc( "@label", "File to import:" ), mPage );
  hbox->addWidget( label );

  mUrlRequester = new KUrlRequester( mPage );
  mUrlRequester->setFilter( "*.csv" );
  hbox->addWidget( mUrlRequester );

  layout->addLayout( hbox, 0, 0, 1, 5 );

  // Delimiter: comma, semicolon, tab, space, other
  QGroupBox *group = new QGroupBox( i18nc( "@title:group", "Delimiter" ), mPage );
  QGridLayout *delimiterLayout = new QGridLayout;
  delimiterLayout->setMargin( marginHint() );
  delimiterLayout->setSpacing( spacingHint() );
  group->setLayout( delimiterLayout );
  delimiterLayout->setAlignment( Qt::AlignTop );
  layout->addWidget( group, 1, 0, 4, 1);

  mDelimiterGroup = new QButtonGroup( this );
  mDelimiterGroup->setExclusive( true );

  mRadioComma = new QRadioButton( i18nc( "@option:radio Field separator", "Comma" ), group );
  mRadioComma->setChecked( true );
  mDelimiterGroup->addButton( mRadioComma, 0 );
  delimiterLayout->addWidget( mRadioComma, 0, 0 );

  mRadioSemicolon = new QRadioButton( i18nc( "@option:radio Field separator", "Semicolon" ), group );
  mDelimiterGroup->addButton( mRadioSemicolon, 1 );
  delimiterLayout->addWidget( mRadioSemicolon, 0, 1 );

  mRadioTab = new QRadioButton( i18nc( "@option:radio Field separator", "Tabulator" ), group );
  mDelimiterGroup->addButton( mRadioTab, 2 );
  delimiterLayout->addWidget( mRadioTab, 1, 0 );

  mRadioSpace = new QRadioButton( i18nc( "@option:radio Field separator", "Space" ), group );
  mDelimiterGroup->addButton( mRadioSpace, 3 );
  delimiterLayout->addWidget( mRadioSpace, 1, 1 );

  mRadioOther = new QRadioButton( i18nc( "@option:radio Custum field separator", "Other" ), group );
  mDelimiterGroup->addButton( mRadioOther, 4 );
  delimiterLayout->addWidget( mRadioOther, 0, 2 );

  mDelimiterEdit = new KLineEdit( group );
  delimiterLayout->addWidget( mDelimiterEdit, 1, 2 );

  mComboLine = new KComboBox( mPage );
  mComboLine->setEditable( false );
  mComboLine->addItem( i18nc( "@item:inlistbox", "1" ) );
  layout->addWidget( mComboLine, 2, 3 );

  mComboQuote = new KComboBox( mPage );
  mComboQuote->setEditable( false );
  mComboQuote->addItem( i18nc( "@item:inlistbox Qoute character option", "\"" ), 0 );
  mComboQuote->addItem( i18nc( "@item:inlistbox Quote character option", "'" ), 1 );
  mComboQuote->addItem( i18nc( "@item:inlistbox Quote character option", "None" ), 2 );
  layout->addWidget( mComboQuote, 2, 2 );

  mDatePatternEdit = new KLineEdit( mPage );
  mDatePatternEdit->setText( "Y-M-D" ); // ISO 8601 format as default
  mDatePatternEdit->setToolTip( i18nc( "@info:tooltip",
                                         "<para><list><item>y: year with 2 digits</item>"
                                         "<item>Y: year with 4 digits</item>"
                                         "<item>m: month with 1 or 2 digits</item>"
                                         "<item>M: month with 2 digits</item>"
                                         "<item>d: day with 1 or 2 digits</item>"
                                         "<item>D: day with 2 digits</item></list></para>" ) );
  layout->addWidget( mDatePatternEdit, 2, 4 );

  label = new QLabel( i18nc( "@label:listbox","Start at line:" ), mPage );
  layout->addWidget( label, 1, 3 );

  label = new QLabel( i18nc( "@label:listbox", "Text quote:" ), mPage );
  layout->addWidget( label, 1, 2 );

  label = new QLabel( i18nc( "@label:listbox", "Date format:" ), mPage );
  layout->addWidget( label, 1, 4 );

  mIgnoreDuplicates = new QCheckBox( mPage );
  mIgnoreDuplicates->setText( i18nc( "@option:check", "Ignore duplicate delimiters" ) );
  layout->addWidget( mIgnoreDuplicates, 3, 2, 1, 3 );

  mCodecCombo = new KComboBox( mPage );
  layout->addWidget( mCodecCombo, 4, 2, 1, 3 );

  mTable = new QTableWidget( 0, 0, mPage );
  mTable->setSelectionMode( QAbstractItemView::NoSelection );
  mTable->setHorizontalHeader( new ComboBoxHeaderView( mTypeMap.keys(), mTable, false ) );
  layout->addWidget( mTable, 5, 0, 1, 5 );

  setButtonText( User1, i18nc( "@action:button", "Apply Template..." ) );
  setButtonText( User2, i18nc( "@action:button", "Save Template..." ) );

  enableButton( Ok, false );
  enableButton( User1, false );
  enableButton( User2, false );

  connect( mTable->horizontalScrollBar(), SIGNAL( valueChanged( int ) ),
           static_cast<ComboBoxHeaderView*>( mTable->horizontalHeader() ), SLOT( adaptMove( int ) ) );

  resize( 400, 300 );
}

void CSVImportDialog::fillTable()
{
  int row, column;
  bool lastCharDelimiter = false;
  bool ignoreDups = mIgnoreDuplicates->isChecked();
  enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD, S_END_OF_QUOTED_FIELD,
         S_MAYBE_NORMAL_FIELD, S_NORMAL_FIELD } state = S_START;

  QChar x;
  QString field;

  // store previous assignment
  QStringList typeStore;
  for ( column = 0; column < mTable->columnCount(); ++column ) {
    if ( mClearTypeStore )
      typeStore << mTypeMap.key( Undefined );
    else {
      ComboBoxHeaderView *view = static_cast< ComboBoxHeaderView* >
          ( mTable->horizontalHeader() );
      int index = view->indexOfHeaderLabel( column );
      typeStore << view->items()[ index ];
    }
  }

  clearTable();

  row = column = 0;

  QTextStream inputStream( mFileArray, QIODevice::ReadOnly );

  // find the current codec
  int code = mCodecCombo->currentIndex();
  if ( code == Local )
    inputStream.setCodec( QTextCodec::codecForLocale() );
  else if ( code >= Codec )
    inputStream.setCodec( mCodecs.at( code - Codec ) );
  else if ( code == Uni )
    inputStream.setCodec("UTF-16");
  else if ( code == MSBug )
    inputStream.setCodec("UTF-16LE");
  else if ( code == Latin1 )
    inputStream.setCodec( "ISO 8859-1" );
  else if ( code == Guess ) {
// following code does not work anymore with Qt4 (QTextCodec::codecForContent returns always a null pointer)
//     QTextCodec* codec = QTextCodec::codecForContent( mFileArray.data(), mFileArray.size() );
//     if ( codec ) {
//       KMessageBox::information( this, i18n( "Using codec '%1'", QLatin1String( codec->name() ) ), i18n( "Encoding" ) );
//       inputStream.setCodec( codec );
//     }
  }

  int maxColumn = 0;
  while ( !inputStream.atEnd() ) {
    inputStream >> x; // read one char

    if ( x == '\r' ) inputStream >> x; // eat '\r', to handle DOS/LOSEDOWS files correctly

    switch ( state ) {
     case S_START :
      if ( x == mTextQuote ) {
        state = S_QUOTED_FIELD;
      } else if ( QString( x ) == mDelimiter ) {
        if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
          ++column;
        lastCharDelimiter = true;
      } else if ( x == '\n' ) {
        ++row;
        column = 0;
      } else {
        field += x;
        state = S_MAYBE_NORMAL_FIELD;
      }
      break;
     case S_QUOTED_FIELD :
      if ( x == mTextQuote ) {
        state = S_MAYBE_END_OF_QUOTED_FIELD;
      } else if ( x == '\n' &&  mTextQuote.isNull() ) {
        setText( row - mStartLine, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 0;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        field += x;
      }
      break;
     case S_MAYBE_END_OF_QUOTED_FIELD :
      if ( x == mTextQuote ) {
        field += x;
        state = S_QUOTED_FIELD;
      } else if ( QString( x ) == mDelimiter || x == '\n' ) {
        setText( row - mStartLine, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 0;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        state = S_END_OF_QUOTED_FIELD;
      }
      break;
     case S_END_OF_QUOTED_FIELD :
      if ( QString( x ) == mDelimiter || x == '\n' ) {
        setText( row - mStartLine, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 0;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        state = S_END_OF_QUOTED_FIELD;
      }
      break;
     case S_MAYBE_NORMAL_FIELD :
      if ( x == mTextQuote ) {
        field = "";
        state = S_QUOTED_FIELD;
        break;
      }
     case S_NORMAL_FIELD :
      if ( QString( x ) == mDelimiter || x == '\n' ) {
        setText( row - mStartLine, column, field );
        field = "";
        if ( x == '\n' ) {
          ++row;
          column = 0;
        } else {
          if ( ( ignoreDups == false ) || ( lastCharDelimiter == false ) )
            ++column;
          lastCharDelimiter = true;
        }
        state = S_START;
      } else {
        field += x;
      }
    }
    if ( QString( x ) != mDelimiter )
      lastCharDelimiter = false;

    if ( column > maxColumn )
      maxColumn = column;
  }

  // file with only one line without '\n'
  if ( field.length() > 0 ) {
    setText( row - mStartLine, column, field );
    ++row;
    field = "";
  }

  adjustRows( row - mStartLine );
  mTable->setColumnCount( maxColumn + 1 );

  QStringList headerLabels( typeStore );

  for ( column = typeStore.count(); column < mTable->columnCount();
        ++column ) {
    headerLabels << mTypeMap.key( Undefined );
  }

  mTable->setHorizontalHeaderLabels( headerLabels );

  resizeColumns();
}

void CSVImportDialog::clearTable()
{
  mTable->clear();
}

void CSVImportDialog::fillComboBox()
{
  mComboLine->clear();
  for ( int row = 0; row < mTable->rowCount(); ++row )
    mComboLine->addItem( QString::number( row ) );
}

void CSVImportDialog::reloadCodecs()
{
  mCodecCombo->clear();

  mCodecs.clear();

  Q_FOREACH( const QByteArray& name, QTextCodec::availableCodecs() ) {
    mCodecs.append( QTextCodec::codecForName( name ) );
  }

  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Local (%1)", QLatin1String( QTextCodec::codecForLocale()->name() ) ), Local );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "[guess]" ), Guess );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Latin1" ), Latin1 );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Unicode" ), Uni );
  mCodecCombo->addItem( i18nc( "@item:inlistbox Codec setting", "Microsoft Unicode" ), MSBug );

  for ( int i = 0; i < mCodecs.count(); ++i )
    mCodecCombo->addItem( mCodecs.at( i )->name(), Codec + i );
}

void CSVImportDialog::setText( int row, int col, const QString& text )
{
  kDebug() << "setText" << row << "," << col << "," << text;

  if ( mTable->rowCount() <= row ) {
    mTable->setRowCount( row + 5000 ); // We add 5000 at a time to limit recalculations
    mAdjustRows = true;
  }

  if ( mTable->columnCount() <= col )
    mTable->setColumnCount( col + 50 ); // We add 50 at a time to limit recalculation

  /*QTableWidgetItem *item = mTable->item( row, col );
  if ( !item ) {
    item = new QTableWidgetItem;
    mTable->setItem( row, col, item );
  }
  item->setText( text );*/
  mTable->setItem( row, col, new QTableWidgetItem( text ) );
}

/*
 * Called after the first fillTable() when number of rows are unknown.
 */
void CSVImportDialog::adjustRows( int rows )
{
  if ( mAdjustRows ) {
    mTable->setRowCount( rows );
    mAdjustRows = false;
  }
}

void CSVImportDialog::resizeColumns()
{
#ifdef __GNUC__
#warning port me!
#endif
  /*QFontMetrics fm = fontMetrics();
  int width = 0;

  QMap<QString, uint>::ConstIterator it;
  for ( it = mTypeMap.begin(); it != mTypeMap.end(); ++it ) {
    width = qMax( width, fm.width( it.key() ) );
  }

  for ( int i = 0; i < mTable->columnCount(); ++i )
    mTable->horizontalHeader->setDefaultSectionSize( i, qMax( width + 15, mTable->columnWidth( i ) ) );*/
}

void CSVImportDialog::returnPressed()
{
  if ( mDelimiterGroup->checkedId() != 4 )
    return;

  mDelimiter = mDelimiterEdit->text();
  fillTable();
}

void CSVImportDialog::textChanged ( const QString& )
{
  mRadioOther->setChecked ( true );
  delimiterClicked( 4 ); // other
}

void CSVImportDialog::delimiterClicked( int id )
{
  switch ( id ) {
    case 0: // comma
      mDelimiter = ",";
      break;
    case 4: // other
      mDelimiterEdit->setFocus( Qt::OtherFocusReason );
      mDelimiter = mDelimiterEdit->text();
      break;
    case 2: // tab
      mDelimiter = "\t";
      break;
    case 3: // space
      mDelimiter = " ";
      break;
    case 1: // semicolon
      mDelimiter = ";";
      break;
  }

  fillTable();
}

void CSVImportDialog::textquoteSelected( const QString& mark )
{
  if ( mComboQuote->currentIndex() == 2 )
    mTextQuote = 0;
  else
    mTextQuote = mark[ 0 ];

  fillTable();
}

void CSVImportDialog::lineSelected( const QString& line )
{
  mStartLine = line.toInt();
  mAdjustRows = true;
  fillTable();
}

void CSVImportDialog::slotOk()
{
  bool assigned = false;

  for ( int column = 0; column < mTable->columnCount(); ++column ) {
    if ( mTypeMap.value( mTable->model()->headerData( column, Qt::Horizontal ).toString() ) != Undefined ) {
      assigned = true;
      break;
    }
  }

  if ( !assigned )
    KMessageBox::sorry( this, i18nc( "@info:status", "You have to assign at least one column." ) );
}

void CSVImportDialog::applyTemplate()
{
  QMap<uint,int> columnMap;
  QMap<QString, QString> fileMap;
  QStringList templates;

  // load all template files
  QStringList list = KGlobal::dirs()->findAllResources( "data" , QString( kapp->objectName() ) +
      "/csv-templates/*.desktop", KStandardDirs::Recursive | KStandardDirs::NoDuplicates );

  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it )
  {
    KConfig config(  *it, KConfig::SimpleConfig);

    if ( !config.hasGroup( "csv column map" ) )
	    continue;

    KConfigGroup group(&config,"Misc");
    templates.append( group.readEntry( "Name" ) );
    fileMap.insert( group.readEntry( "Name" ), *it );
  }

  // let the user chose, what to take
  bool ok = false;
  QString tmp;
  tmp = KInputDialog::getItem( i18nc( "@title:window", "Template Selection" ),
                  i18nc( "@info", "Please select a template, that matches the CSV file:" ),
                  templates, 0, false, &ok, this );

  if ( !ok )
    return;

  KConfig _config( fileMap[ tmp ], KConfig::SimpleConfig );
  KConfigGroup config(&_config, "General" );
  mDatePatternEdit->setText( config.readEntry( "DatePattern", "Y-M-D" ) );
  uint numColumns = config.readEntry( "Columns", 0 );
  mDelimiterEdit->setText( config.readEntry( "DelimiterOther" ) );
  mDelimiterGroup->button( config.readEntry( "DelimiterType", 0) )->setChecked( true );
  delimiterClicked( config.readEntry( "DelimiterType",0 ) );
  int quoteType = config.readEntry( "QuoteType", 0 );
  mComboQuote->setCurrentIndex( quoteType );
  textquoteSelected( mComboQuote->currentText() );

  // create the column map
  config.changeGroup( "csv column map" );
  for ( uint i = 0; i < numColumns; ++i ) {
    int col = config.readEntry( QString::number( i ),0 );
    columnMap.insert( i, col );
  }

  // apply the column map
  QStringList headerLabels;

  for ( int column = 0; column < columnMap.count(); ++column ) {
    int type = columnMap[ column ];
    headerLabels << mTypeMap.key( type );
  }

  mTable->setHorizontalHeaderLabels( headerLabels );
}

void CSVImportDialog::saveTemplate()
{
  QString fileName = KFileDialog::getSaveFileName(
                     KStandardDirs::locateLocal( "data", QString( kapp->objectName() ) + "/csv-templates/" ),
                     "*.desktop", this );

  if ( fileName.isEmpty() )
    return;

  if ( !fileName.contains( ".desktop" ) )
    fileName += ".desktop";

  QString name = KInputDialog::getText( i18nc( "@title:window", "Template Name" ),
                                        i18nc( "@info", "Please enter a name for the template:" ) );

  if ( name.isEmpty() )
    return;

  KConfig _config( fileName  );
  KConfigGroup config(&_config, "General" );
  config.writeEntry( "DatePattern", mDatePatternEdit->text() );
  config.writeEntry( "Columns", mTable->columnCount() );
  config.writeEntry( "DelimiterType", mDelimiterGroup->checkedId() );
  config.writeEntry( "DelimiterOther", mDelimiterEdit->text() );
  config.writeEntry( "QuoteType", mComboQuote->currentIndex() );

  KConfigGroup miscGrp(&_config, "Misc" );
  miscGrp.writeEntry( "Name", name );

  KConfigGroup cvsGrp(&_config, "csv column map" );

  ComboBoxHeaderView *headerView = static_cast< ComboBoxHeaderView* >
      ( mTable->horizontalHeader() );
  for ( int column = 0; column < mTable->columnCount(); ++column ) {
    cvsGrp.writeEntry( QString::number( column ), mTypeMap.value( mTable->model()->headerData( column, Qt::Horizontal ).toString() ) );
  }

  _config.sync();
}

QString CSVImportDialog::getText( int row, int col )
{
  QTableWidgetItem *item = mTable->item( row, col );
  if ( item ) {
    return item->text();
  } else {
    return QString();
  }
}

uint CSVImportDialog::posToType( int pos ) const
{
  uint counter = 0;
  QMap<QString, uint>::ConstIterator it;
  for ( it = mTypeMap.begin(); it != mTypeMap.end(); ++it, ++counter )
    if ( counter == (uint)pos )
      return it.value();

  return 0;
}

int CSVImportDialog::typeToPos( uint type ) const
{
  uint counter = 0;
  QMap<QString, uint>::ConstIterator it;
  for ( it = mTypeMap.begin(); it != mTypeMap.end(); ++it, ++counter )
    if ( it.value() == type )
      return counter;

  return -1;
}

void CSVImportDialog::ignoreDuplicatesChanged( int )
{
  fillTable();
}

void CSVImportDialog::setFile( const KUrl &fileName )
{
   setFile(fileName.toLocalFile());
}

void CSVImportDialog::setFile( const QString &fileName )
{
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    KMessageBox::sorry( this, i18nc( "@info:status", "Cannot open input file." ) );
    file.close();
    return;
  }

  mFileArray = file.readAll();
  file.close();

  mClearTypeStore = true;
  clearTable();
  mTable->setColumnCount( 0 );
  mTable->setRowCount( 0 );
  fillTable();
  mClearTypeStore = false;

  fillComboBox();
}

void CSVImportDialog::urlChanged( const QString &file )
{
  bool state = !file.isEmpty();

  enableButton( Ok, state );
  enableButton( User1, state );
  enableButton( User2, state );
}

void CSVImportDialog::codecChanged()
{
  fillTable();
}

#include <csvimportdialog.moc>
