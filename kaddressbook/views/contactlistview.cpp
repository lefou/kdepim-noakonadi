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

#include "contactlistview.h"
#include <QToolTip>

#include <Qt3Support/Q3Header>
#include <QtGui/QBrush>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <k3listview.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kapplication.h>
#include <kcolorscheme.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kimproxy.h>
#include <klocale.h>
#include <kurl.h>

#include "kaddressbooktableview.h"

///////////////////////////
// ContactListViewItem Methods

ContactListViewItem::ContactListViewItem(const KABC::Addressee &a,
                                         ContactListView *parent,
                                         KABC::AddressBook *doc,
                                         const KABC::Field::List &fields,
                                         KIMProxy *proxy )
  : K3ListViewItem(parent), mAddressee(a), mFields( fields ),
    parentListView( parent ), mDocument(doc), mIMProxy( proxy )
{
  if ( mIMProxy )
    mHasIM = mIMProxy->isPresent( mAddressee.uid() );
  else
    mHasIM = false;
  refresh();
}

QString ContactListViewItem::key(int column, bool ascending) const
{
  // Preserve behaviour of QListViewItem::key(), otherwise we cause a crash if the column does not exist
  if ( column >= parentListView->columns() )
    return QString();

  Q_UNUSED( ascending )
  if ( parentListView->showIM() ) {
    // in this case, one column is reserved for IM presence
    // so we have to process it differently
    if ( column == parentListView->imColumn() ) {
      // increment by one before converting to string so that -1 is not greater than 1
      // create the sort key by taking the numeric status 0 low, 5 high, and subtracting it from 5
      // so that the default ascending gives online before offline, etc.
      QString key = QString::number( 5 - ( mIMProxy->presenceNumeric( mAddressee.uid() ) + 1 ) );
      return key;
    }
    else {
      return mFields[ column ]->sortKey( mAddressee );
    }
  }
  else
    return mFields[ column ]->sortKey( mAddressee );
}

void ContactListViewItem::paintCell(QPainter * p,
                                    const QColorGroup & cg,
                                    int column,
                                    int width,
                                    int align)
{
  K3ListViewItem::paintCell(p, cg, column, width, align);

  if ( !p )
    return;

  if (parentListView->singleLine()) {
    p->setPen( parentListView->alternateColor() );
    p->drawLine( 0, height() - 1, width, height() - 1 );
  }
}


ContactListView *ContactListViewItem::parent()
{
  return parentListView;
}


void ContactListViewItem::refresh()
{
  if ( !mDocument ) {
    return;
  }

  // Update our addressee, since it may have changed else were
  mAddressee = mDocument->findByUid( mAddressee.uid() );
  if ( mAddressee.isEmpty() ) {
    return;
  }

  int i = 0;
  // don't show unknown presence, it's not interesting
  if ( mHasIM ) {
    if ( mIMProxy->presenceNumeric( mAddressee.uid() ) > 0 )
      setPixmap( parentListView->imColumn(), mIMProxy->presenceIcon( mAddressee.uid() ) );
    else
      setPixmap( parentListView->imColumn(), QPixmap() );
  }

  KABC::Field::List::ConstIterator it;
  for ( it = mFields.constBegin(); it != mFields.constEnd(); ++it ) {
    if ( (*it)->label() == KABC::Addressee::birthdayLabel() ) {
      QDate date = mAddressee.birthday().date();
      if ( date.isValid() )
        setText( i++, KGlobal::locale()->formatDate( date, KLocale::ShortDate ) );
      else
        setText( i++, "" );
    } else
      setText( i++, (*it)->value( mAddressee ) );
  }
}

void ContactListViewItem::setHasIM( bool hasIM )
{
  mHasIM = hasIM;
}

///////////////////////////////
// ContactListView

ContactListView::ContactListView(KAddressBookTableView *view,
                                 KABC::AddressBook* /* doc */,
                                 QWidget *parent,
                                 const char *name )
  : K3ListView( parent ),
    pabWidget( view ),
    oldColumn( 0 )
{
  setObjectName(name);
  mABackground = true;
  mSingleLine = false;
  mToolTips = true;
  mShowIM = true;
  mAlternateColor = KColorScheme( QPalette::Active, KColorScheme::View ).background( KColorScheme::AlternateBackground ).color();

  setAlternateBackgroundEnabled(mABackground);
  setAcceptDrops( true );
  viewport()->setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setShowSortIndicator(true);
  setSelectionModeExt( K3ListView::Extended );
  setDropVisualizer(false);
  connect(this, SIGNAL(dropped(QDropEvent*)),
          this, SLOT(itemDropped(QDropEvent*)));
}


bool ContactListView::event( QEvent *e )
{

  if( e->type() != QEvent::ToolTip )
    return K3ListView::event( e );
  if ( !tooltips() )
    return true;
  QHelpEvent * he = static_cast< QHelpEvent * >( e );
  QPoint pnt = viewport()->mapFromGlobal( mapToGlobal( he->pos() ) );
  Q3ListViewItem * item = itemAt ( pnt );
  if ( item )
  {
    ContactListViewItem *plvi = static_cast<ContactListViewItem *>( item );
    QString s;

    //kDebug(5720) <<"Tip rec:" << r.x() <<"," << r.y() <<"," << r.width()
    //          << "," << r.height();

    KABC::Addressee a = plvi->addressee();
    if (a.isEmpty())
      return true;

    s += i18nc("label: value", "%1: %2", a.formattedNameLabel(),
               a.formattedName());

    s += '\n';
    s += i18nc("label: value", "%1: %2", a.organizationLabel(),
               a.organization());

    QString notes = a.note().trimmed();
    if ( !notes.isEmpty() ) {
      notes += '\n';
      s += '\n' + i18nc("label: value", "%1: \n", a.noteLabel());
      QFontMetrics fm( font() );

      // Begin word wrap code based on QMultiLineEdit code
      int i = 0;
      bool doBreak = false;
      int linew = 0;
      int lastSpace = -1;
      int a = 0;
      int lastw = 0;

      while ( i < int(notes.length()) ) {
        doBreak = false;
        if ( notes[i] != '\n' )
          linew += fm.width( notes[i] );

        if ( lastSpace >= a && notes[i] != '\n' )
          if  (linew >= parentWidget()->width()) {
            doBreak = true;
            if ( lastSpace > a ) {
              i = lastSpace;
              linew = lastw;
            }
            else
              i = qMax( a, i-1 );
          }

        if ( notes[i] == '\n' || doBreak ) {
          s += notes.mid( a, i - a + (doBreak?1:0) ) +'\n';

          a = i + 1;
          lastSpace = a;
          linew = 0;
        }

        if ( notes[i].isSpace() ) {
          lastSpace = i;
          lastw = linew;
        }

        if ( lastSpace <= a ) {
          lastw = linew;
        }

        ++i;
      }
    }
    if ( s.isEmpty() )
      QToolTip::hideText();
    else
      QToolTip::showText( he->globalPos(), s );
  }
  return true;
}

void ContactListView::paintEmptyArea( QPainter * p, const QRect & rect )
{
  QBrush b = palette().brush(QPalette::Active, QPalette::Base);

  // Get the brush, which will have the background pixmap if there is one.
  if (!b.texture().isNull())
  {
    p->drawTiledPixmap( rect.left(), rect.top(), rect.width(), rect.height(),
      b.texture(),
      rect.left() + contentsX(),
      rect.top() + contentsY() );
  }

  else
  {
    // Do a normal paint
    K3ListView::paintEmptyArea(p, rect);
  }
}

void ContactListView::contentsMousePressEvent(QMouseEvent* e)
{
  presspos = e->pos();
  K3ListView::contentsMousePressEvent(e);
}


// To initiate a drag operation
void ContactListView::contentsMouseMoveEvent( QMouseEvent *e )
{
  if ((e->buttons() & Qt::LeftButton) && (e->pos() - presspos).manhattanLength() > 4 ) {
    emit startAddresseeDrag();
  }
  else
    K3ListView::contentsMouseMoveEvent( e );
}

bool ContactListView::acceptDrag(QDropEvent *e) const
{
  return e->mimeData()->hasText();
}

void ContactListView::itemDropped(QDropEvent *e)
{
  contentsDropEvent(e);
}

void ContactListView::contentsDropEvent( QDropEvent *e )
{
  emit addresseeDropped(e);
}

void ContactListView::setAlternateBackgroundEnabled(bool enabled)
{
  mABackground = enabled;

  if (mABackground)
  {
    setAlternateBackground(mAlternateColor);
  }
  else
  {
    setAlternateBackground(QColor());
  }
}

void ContactListView::setBackgroundPixmap(const QString &filename)
{
  if (filename.isEmpty())
  {
    setPalette( QPalette() );
  }
  else
  {
    QPalette palette;
    palette.setBrush( backgroundRole(), QBrush(QPixmap(filename) ));
    setPalette( palette );
  }
}

void ContactListView::setShowIM( bool enabled )
{
  mShowIM = enabled;
}

bool ContactListView::showIM()
{
  return mShowIM;
}

void ContactListView::setIMColumn( int column )
{
  mInstantMsgColumn = column;
}

int ContactListView::imColumn()
{
  return mInstantMsgColumn;
}

#include "contactlistview.moc"
