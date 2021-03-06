/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#include "kaddressbookcardview.h"

#include <QtCore/QEvent>
#include <QtCore/QStringList>
#include <QtGui/QApplication>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QVBoxLayout>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>

#include "core.h"
#include "configurecardviewdialog.h"
#include "kabprefs.h"

class CardViewFactory : public ViewFactory
{
  public:
    KAddressBookView *view( KAB::Core *core, QWidget *parent )
    {
      return new KAddressBookCardView( core, parent );
    }

    QString type() const { return I18N_NOOP("Card"); }

    QString description() const { return i18n( "Rolodex style cards represent contacts." ); }

    ViewConfigureWidget *configureWidget( KABC::AddressBook *ab, QWidget *parent )
    {
      return new ConfigureCardViewWidget( ab, parent );
    }
};

K_EXPORT_PLUGIN(CardViewFactory)

class AddresseeCardViewItem : public CardViewItem
{
  public:
    AddresseeCardViewItem( const KABC::Field::List &fields,
                           bool showEmptyFields,
                           KABC::AddressBook *doc, const KABC::Addressee &addr,
                           CardView *parent )
      : CardViewItem( parent, addr.realName() ),
        mFields( fields ), mShowEmptyFields( showEmptyFields ),
        mDocument( doc ), mAddressee( addr )
      {
        if ( mFields.isEmpty() )
          mFields = KABC::Field::defaultFields();

        refresh();
      }

    const KABC::Addressee &addressee() const { return mAddressee; }

    void refresh()
    {
      mAddressee = mDocument->findByUid( mAddressee.uid() );

      if ( !mAddressee.isEmpty() ) {
        clearFields();

        KABC::Field::List::ConstIterator it( mFields.begin() );
        const KABC::Field::List::ConstIterator endIt( mFields.end() );
        for ( ; it != endIt; ++it ) {
          // insert empty fields or not? not doing so saves a bit of memory and CPU
          // (during geometry calculations), but prevents having equally
          // wide label columns in all cards, unless CardViewItem/CardView search
          // globally for the widest label. (anders)

          // if ( mShowEmptyFields || !(*it)->value( mAddressee ).isEmpty() )
          insertField( (*it)->label(), (*it)->value( mAddressee ) );
        }

        setCaption( mAddressee.realName() );
      }
    }

  private:
    KABC::Field::List mFields;
    bool mShowEmptyFields;
    KABC::AddressBook *mDocument;
    KABC::Addressee mAddressee;
};


AddresseeCardView::AddresseeCardView( QWidget *parent, const char *name )
  : CardView( parent, name )
{
  setAcceptDrops( true );
}

AddresseeCardView::~AddresseeCardView()
{
}

void AddresseeCardView::dragEnterEvent( QDragEnterEvent *event )
{
  const QMimeData *md = event->mimeData();
  if ( md->hasText() )
    event->accept();
}

void AddresseeCardView::dropEvent( QDropEvent *event )
{
  emit addresseeDropped( event );
}

void AddresseeCardView::startDrag()
{
  emit startAddresseeDrag();
}


KAddressBookCardView::KAddressBookCardView( KAB::Core *core,
                                            QWidget *parent )
    : KAddressBookView( core, parent )
{
  mShowEmptyFields = false;

  QVBoxLayout *layout = new QVBoxLayout( viewWidget() );
  layout->setMargin( 0 );

  mCardView = new AddresseeCardView( viewWidget(), "mCardView" );
  mCardView->setSelectionMode( CardView::Extended );
  layout->addWidget( mCardView );

  // Connect up the signals
  connect( mCardView, SIGNAL( executed( CardViewItem* ) ),
           this, SLOT( addresseeExecuted( CardViewItem* ) ) );
  connect( mCardView, SIGNAL( selectionChanged() ),
           this, SLOT( addresseeSelected() ) );
  connect( mCardView, SIGNAL( addresseeDropped( QDropEvent* ) ),
           this, SIGNAL( dropped( QDropEvent* ) ) );
  connect( mCardView, SIGNAL( startAddresseeDrag() ),
           this, SIGNAL( startDrag() ) );
  connect( mCardView, SIGNAL( contextMenuRequested( CardViewItem*, const QPoint& ) ),
           this, SLOT( rmbClicked( CardViewItem*, const QPoint& ) ) );
}

KAddressBookCardView::~KAddressBookCardView()
{
}

KABC::Field *KAddressBookCardView::sortField() const
{
  // we have hardcoded sorting, so we have to return a hardcoded field :(
  return KABC::Field::allFields()[ 0 ];
}

void KAddressBookCardView::readConfig( KConfigGroup &cfg )
{
  KAddressBookView::readConfig( cfg );

  // costum colors?
  if ( cfg.readEntry( "EnableCustomColors", false ) ) {
    QPalette p( mCardView->palette() );
    QColor c = p.color( QPalette::Normal, QPalette::Base );
    p.setColor( QPalette::Normal, QPalette::Base, cfg.readEntry( "BackgroundColor", c ) );
    c = p.color( QPalette::Normal, QPalette::Text );
    p.setColor( QPalette::Normal, QPalette::Text, cfg.readEntry( "TextColor", c ) );
    c = p.color( QPalette::Normal, QPalette::Button );
    p.setColor( QPalette::Normal, QPalette::Button, cfg.readEntry( "HeaderColor", c ) );
    c = p.color( QPalette::Normal, QPalette::ButtonText );
    p.setColor( QPalette::Normal, QPalette::ButtonText, cfg.readEntry( "HeaderTextColor", c ) );
    c = p.color( QPalette::Normal, QPalette::Highlight );
    p.setColor( QPalette::Normal, QPalette::Highlight, cfg.readEntry( "HighlightColor", c ) );
    c = p.color( QPalette::Normal, QPalette::HighlightedText );
    p.setColor( QPalette::Normal, QPalette::HighlightedText, cfg.readEntry( "HighlightedTextColor", c ) );
    mCardView->viewport()->setPalette( p );
  } else {
    // needed if turned off during a session.
    mCardView->viewport()->setPalette( mCardView->palette() );
  }

  //custom fonts?
  QFont f( font() );
  if ( cfg.readEntry( "EnableCustomFonts", false ) ) {
    mCardView->setFont( cfg.readEntry( "TextFont", f ) );
    f.setBold( true );
    mCardView->setHeaderFont( cfg.readEntry( "HeaderFont", f ) );
  } else {
    mCardView->setFont( f );
    f.setBold( true );
    mCardView->setHeaderFont( f );
  }

  mCardView->setDrawCardBorder( cfg.readEntry( "DrawBorder", true ) );
  mCardView->setDrawColSeparators( cfg.readEntry( "DrawSeparators", true ) );
  mCardView->setDrawFieldLabels( cfg.readEntry( "DrawFieldLabels", false ) );
  mShowEmptyFields = cfg.readEntry( "ShowEmptyFields", false );

  mCardView->setShowEmptyFields( mShowEmptyFields );

  mCardView->setItemWidth( cfg.readEntry( "ItemWidth", 200 ) );
  mCardView->setItemMargin( cfg.readEntry( "ItemMargin", 0 ) );
  mCardView->setItemSpacing( cfg.readEntry( "ItemSpacing", 10 ) );
  mCardView->setSeparatorWidth( cfg.readEntry( "SeparatorWidth", 2 ) );

  disconnect( mCardView, SIGNAL( executed( CardViewItem* ) ),
              this, SLOT( addresseeExecuted( CardViewItem* ) ) );

  if ( KABPrefs::instance()->honorSingleClick() )
    connect( mCardView, SIGNAL( executed( CardViewItem* ) ),
             this, SLOT( addresseeExecuted( CardViewItem* ) ) );
  else
    connect( mCardView, SIGNAL( doubleClicked( CardViewItem* ) ),
             this, SLOT( addresseeExecuted( CardViewItem* ) ) );
}

void KAddressBookCardView::writeConfig( KConfigGroup &cfg )
{
  cfg.writeEntry( "ItemWidth", mCardView->itemWidth() );
  KAddressBookView::writeConfig( cfg );
}

QStringList KAddressBookCardView::selectedUids()
{
  QStringList uidList;
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  for ( item = mCardView->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );
      if ( aItem )
        uidList << aItem->addressee().uid();
    }
  }

  return uidList;
}

void KAddressBookCardView::refresh( const QString &uid )
{
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  if ( uid.isEmpty() ) {
    // Rebuild the view
    mCardView->viewport()->setUpdatesEnabled( false );
    mCardView->clear();

    const KABC::Addressee::List addresseeList( addressees() );
    KABC::Addressee::List::ConstIterator it( addresseeList.begin() );
    const KABC::Addressee::List::ConstIterator endIt( addresseeList.end() );
    for ( ; it != endIt; ++it ) {
      aItem = new AddresseeCardViewItem( fields(), mShowEmptyFields,
                                         core()->addressBook(), *it, mCardView );
    }
    mCardView->viewport()->setUpdatesEnabled( true );
    mCardView->viewport()->update();

    // by default nothing is selected
    emit selected( QString() );
  } else {
    // Try to find the one to refresh
    bool found = false;
    for ( item = mCardView->firstItem(); item && !found; item = item->nextItem() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );
      if ( aItem && (aItem->addressee().uid() == uid) ) {
        aItem->refresh();
        found = true;
      }
    }
  }
}

void KAddressBookCardView::setSelected( const QString &uid, bool selected )
{
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  if ( uid.isEmpty() ) {
    mCardView->selectAll( selected );
  } else {
    bool found = false;
    for ( item = mCardView->firstItem(); item && !found; item = item->nextItem() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );

      if ( aItem && (aItem->addressee().uid() == uid) ) {
        mCardView->setSelected( aItem, selected );
        mCardView->ensureItemVisible( item );
        found = true;
      }
    }
  }
}

void KAddressBookCardView::setFirstSelected( bool selected )
{
  if ( mCardView->firstItem() ) {
    mCardView->setSelected( mCardView->firstItem(), selected );
    mCardView->ensureItemVisible( mCardView->firstItem() );
  }
}

void KAddressBookCardView::addresseeExecuted( CardViewItem *item )
{
  AddresseeCardViewItem *aItem = dynamic_cast<AddresseeCardViewItem*>( item );
  if ( aItem )
    emit executed( aItem->addressee().uid() );
}

void KAddressBookCardView::addresseeSelected()
{
  CardViewItem *item;
  AddresseeCardViewItem *aItem;

  bool found = false;
  for ( item = mCardView->firstItem(); item && !found; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      aItem = dynamic_cast<AddresseeCardViewItem*>( item );
      if ( aItem ) {
        emit selected( aItem->addressee().uid() );
        found = true;
      }
    }
  }

  if ( !found )
    emit selected( QString() );
}

void KAddressBookCardView::rmbClicked( CardViewItem*, const QPoint &point )
{
  popup( point );
}

void KAddressBookCardView::scrollUp()
{
  QApplication::postEvent( mCardView, new QKeyEvent( QEvent::KeyPress, Qt::Key_Up, 0 ) );
}

void KAddressBookCardView::scrollDown()
{
  QApplication::postEvent( mCardView, new QKeyEvent( QEvent::KeyPress, Qt::Key_Down, 0 ) );
}

#include "kaddressbookcardview.moc"
