/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KOAGENDA_H
#define KOAGENDA_H

#include "koeventview.h"
#include <kcal/incidence.h>

#include <q3scrollview.h>
#include <QFrame>
#include <QList>
#include <QPointer>
#include <QTimer>
#include <QVector>

class KOAgenda;
class KOAgendaItem;

class QDropEvent;
class QEvent;
class QKeyEvent;
class QLabel;
class QMouseEvent;
class QResizeEvent;
class QTime;
class QWheelEvent;

namespace KOrg {
  class IncidenceChangerBase;
}
using namespace KOrg;

namespace KCal {
  class Event;
  class Todo;
  class Calendar;
}
using namespace KCal;

class MarcusBains : public QFrame
{
  Q_OBJECT
  public:
    MarcusBains( KOAgenda *agenda = 0 );
    void updateLocationRecalc( bool recalculate = false );
    virtual ~MarcusBains();

  public slots:
    void updateLocation();

  private:
    int todayColumn();
    QTimer *mTimer;
    QLabel *mTimeBox;  // Label showing the current time
    KOAgenda *mAgenda;
    QTime mOldTime;
    int mOldTodayCol;
};

class KOAgenda : public Q3ScrollView
{
  Q_OBJECT
  public:
    KOAgenda ( KOEventView *eventView, int columns, int rows, int columnSize,
               QWidget *parent=0, Qt::WFlags f = 0 );
    explicit KOAgenda ( KOEventView *eventView, int columns,
                        QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~KOAgenda();

    Incidence *selectedIncidence() const;
    QDate selectedIncidenceDate() const;

    /**
      Returns the uid of the last incidence that was selected. This
      persists across reloads and clear, so that if the same uid
      reappears, it can be reselected.
    */
    const QString lastSelectedUid() const;

    bool eventFilter ( QObject *, QEvent * );

    QPoint contentsToGrid ( const QPoint &pos ) const;
    QPoint gridToContents ( const QPoint &gpos ) const;

    int timeToY ( const QTime &time ) const;
    QTime gyToTime ( int y ) const;

    QVector<int> minContentsY() const;
    QVector<int> maxContentsY() const;

    int visibleContentsYMin();
    int visibleContentsYMax();

    void setStartTime( const QTime &startHour );

    KOAgendaItem *insertItem ( Incidence *incidence, const QDate &qd, int X, int YTop,
                               int YBottom );
    KOAgendaItem *insertAllDayItem ( Incidence *event, const QDate &qd, int XBegin,
                                     int XEnd );
    void insertMultiItem ( Event *event, const QDate &qd, int XBegin, int XEnd,
                           int YTop, int YBottom );

    /**
      Removes an event and all its multi-items from the agenda. This function
      removes the items from the view, but doesn't delete them immediately.
      Instead, they are queued in mItemsToDelete and later deleted by the
      slot deleteItemsToDelete() (called by QTimer::singleShot ).
      @param incidence The pointer to the incidence that should be removed.
    */
    void removeIncidence( Incidence *incidence );

    void changeColumns( int columns );

    int columns() { return mColumns; }
    int rows() { return mRows; }

    double gridSpacingX() const { return mGridSpacingX; }
    double gridSpacingY() const { return mGridSpacingY; }
    void clear();

    /** Calculates the minimum width */
    virtual int minimumWidth() const;
    /** Update configuration from preference settings */
    void updateConfig();

    void checkScrollBoundaries();

    void setHolidayMask( QVector<bool> * );

    void setDateList( const DateList &selectedDates );
    DateList dateList() const;

    void setCalendar( Calendar *cal )
    { mCalendar = cal; }
    void setIncidenceChanger( IncidenceChangerBase *changer )
    { mChanger = changer; }

  public slots:
    void scrollUp();
    void scrollDown();

    void checkScrollBoundaries( int );

    /** Deselect selected items. This function does not emit any signals. */
    void deselectItem();

    void clearSelection();

    /**
      Select item. If the argument is 0, the currently selected item gets
      deselected. This function emits the itemSelected(bool) signal to inform
      about selection/deselection of events.
    */
    void selectItem( KOAgendaItem * );

    /**
      Selects the item associated with a given uid.
      Linear search, use carefully.
      @param uid the UID of the item that should be selected. If no such
      item exists, the selection is not changed.
    */
    void selectItemByUID( const QString &uid );
    bool removeAgendaItem( KOAgendaItem *item );
    void showAgendaItem( KOAgendaItem *item );

  signals:
    void newEventSignal();
    void newTimeSpanSignal( const QPoint &, const QPoint & );
    void newStartSelectSignal();

    void showIncidenceSignal( Incidence * );
    void editIncidenceSignal( Incidence * );
    void deleteIncidenceSignal( Incidence * );
    void showIncidencePopupSignal( Calendar *, Incidence *, const QDate &);
    void showNewEventPopupSignal();

    void itemModified( KOAgendaItem *item );
    void incidenceSelected( Incidence *, const QDate & );
    void startMultiModify( const QString & );
    void endMultiModify();

    void lowerYChanged( int );
    void upperYChanged( int );

    void startDragSignal( Incidence * );
    void droppedToDo( Todo *todo, const QPoint &gpos, bool allDay );

    void enableAgendaUpdate( bool enable );
    void zoomView( const int delta, const QPoint &pos, const Qt::Orientation );

    void mousePosSignal( const QPoint &pos );
    void enterAgenda();
    void leaveAgenda();

    void gridSpacingYChanged( double );

  private:
    enum MouseActionType {
      NOP,
      MOVE,
      SELECT,
      RESIZETOP,
      RESIZEBOTTOM,
      RESIZELEFT,
      RESIZERIGHT
    };

  protected:
    /**
      Reimp from Q3ScrollView: Draw the background grid of the agenda.
      @p cw grid width
      @p ch grid height
    */
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    //the following line is here to shutup gcc warning:
    //‘virtual void Q3ScrollView::drawContents(QPainter*)’ was hidden
    using Q3Frame::drawContents;

    int columnWidth( int column ) const;
    virtual void resizeEvent ( QResizeEvent * );

    /** Handles mouse events. Called from eventFilter */
    virtual bool eventFilter_mouse ( QObject *, QMouseEvent * );
#ifndef QT_NO_WHEELEVENT
    /** Handles mousewheel events. Called from eventFilter */
    virtual bool eventFilter_wheel ( QObject *, QWheelEvent * );
#endif
    /** Handles key events. Called from eventFilter */
    virtual bool eventFilter_key ( QObject *, QKeyEvent * );

    /** Handles drag and drop events. Called from eventFilter */
    virtual bool eventFilter_drag( QObject *, QDropEvent * );

    /** returns RESIZELEFT if pos is near the lower edge of the action item,
      RESIZERIGHT if pos is near the higher edge, and MOVE otherwise.
      If --reverse is used, RESIZELEFT still means resizing the beginning of
      the event, although that means moving to the right!
      horizontal is the same as mAllDayAgenda.
        @param horizontal Whether horizontal resizing is  possible
        @param pos The current mouse position
        @param item The affected item
    */
    MouseActionType isInResizeArea( bool horizontal, const QPoint &pos, KOAgendaItem *item );
    /** Return whether the cell specified by the grid point belongs to the current select
    */
    bool ptInSelection( const QPoint &gpos ) const;

    /** Start selecting time span. */
    void startSelectAction( const QPoint &viewportPos );

    /** Select time span. */
    void performSelectAction( const QPoint &viewportPos );

    /** Emd selecting time span. */
    void endSelectAction( const QPoint &viewportPos );

    /** Start moving/resizing agenda item */
    void startItemAction( const QPoint &viewportPos );

    /** Move/resize agenda item */
    void performItemAction( const QPoint &viewportPos );

    /** End moving/resizing agenda item */
    void endItemAction();

    /** Set cursor, when no item action is in progress */
    void setNoActionCursor( KOAgendaItem *moveItem, const QPoint &viewportPos );
    /** Sets the cursor according to the given action type.
        @param actionType The type of action for which the cursor should be set.
        @param acting If true, the corresponding action is running (e.g. the
        item is currently being moved by the user). If false the
        cursor should just indicate that the corresponding
        action is possible */
    void setActionCursor( int actionType, bool acting=false );

    /** calculate the width of the column subcells of the given item */
    double calcSubCellWidth( KOAgendaItem *item );
    /** Move and resize the given item to the correct position */
    void placeAgendaItem( KOAgendaItem *item, double subCellWidth );
    /** Place agenda item in agenda and adjust other cells if necessary */
    void placeSubCells( KOAgendaItem *placeItem );
    /** Place the agenda item at the correct position (ignoring conflicting items) */
    void adjustItemPosition( KOAgendaItem *item );

    /** Process the keyevent, including the ignored keyevents of eventwidgets.
     * Implements pgup/pgdn and cursor key navigation in the view.
     */
    void keyPressEvent( QKeyEvent * );

    void calculateWorkingHours();

    virtual void contentsMousePressEvent ( QMouseEvent * );

    void emitNewEventForSelection();

  protected slots:
    /** delete the items that are queued for deletion */
    void deleteItemsToDelete();
    /** Resizes all the child elements after the size of the agenda
        changed. This is needed because Qt seems to have a bug when
        the resizeEvent of one of the widgets in a splitter takes a
        lot of time / does a lot of resizes.... see bug 80114 */
    void resizeAllContents();

  private:
    void init();
    void marcus_bains();
    bool mAllDayMode;

    // We need the calendar for drag'n'drop and for paint the ResourceColor
    Calendar *mCalendar;

    // Width and height of agenda cells. mDesiredGridSpacingY is the height
    // set in the config. The actual height might be larger since otherwise
    // more than 24 hours might be displayed.
    double mGridSpacingX;
    double mGridSpacingY;
    double mDesiredGridSpacingY;

    // size of border, where mouse action will resize the KOAgendaItem
    int mResizeBorderWidth;

    // size of border, where mouse mve will cause a scroll of the agenda
    int mScrollBorderWidth;
    int mScrollDelay;
    int mScrollOffset;

    QTimer mScrollUpTimer;
    QTimer mScrollDownTimer;

    // Number of Columns/Rows of agenda grid
    int mColumns;
    int mRows;

    // Cells to store Move and Resize coordiantes while performing the action
    QPoint mStartCell;
    QPoint mEndCell;

    // Working Hour coordiantes
    bool mWorkingHoursEnable;
    QVector<bool> *mHolidayMask;
    int mWorkingHoursYTop;
    int mWorkingHoursYBottom;

    // Selection
    bool mHasSelection;
    QPoint mSelectionStartPoint;
    QPoint mSelectionStartCell;
    QPoint mSelectionEndCell;

    // List of dates to be displayed
    DateList mSelectedDates;

    // The KOAgendaItem, which has been right-clicked last
    QPointer<KOAgendaItem> mClickedItem;

    // The KOAgendaItem, which is being moved/resized
    QPointer<KOAgendaItem> mActionItem;

    // Currently selected item
    QPointer<KOAgendaItem> mSelectedItem;
    // Uid of the last selected item. Used for reselecting in situations
    // where the selected item points to a no longer valid incidence, for
    // example during resource reload.
    QString mSelectedUid;

    // The Marcus Bains Line widget.
    MarcusBains *mMarcusBains;

    MouseActionType mActionType;

    bool mItemMoved;

    // List of all Items contained in agenda
    QList<KOAgendaItem*> mItems;
    QList<KOAgendaItem*> mItemsToDelete;

    int mOldLowerScrollValue;
    int mOldUpperScrollValue;

    bool mReturnPressed;
    KOrg::IncidenceChangerBase *mChanger;

    KOEventView *mEventView;
};

#endif // KOAGENDA_H
