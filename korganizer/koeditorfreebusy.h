/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOEDITORFREEBUSY_H
#define KOEDITORFREEBUSY_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>
//Added by qt3to4:
#include <QLabel>
#include <QTimerEvent>

class QLabel;
class KDGanttView;
class KDGanttViewItem;
class FreeBusyItem;

namespace KCal {
  class FreeBusy;
  class Attendee;
}


class KOEditorFreeBusy : public QWidget
{
    Q_OBJECT
  public:
    KOEditorFreeBusy( int spacing = 8, QWidget *parent = 0 );
    virtual ~KOEditorFreeBusy();

    void setUpdateEnabled( bool enabled );
    bool updateEnabled() const;

    void insertAttendee( KCal::Attendee *, bool readFBList );
    void removeAttendee( KCal::Attendee * );
    void updateAttendee( KCal::Attendee * );
    void clearAttendees();

    void readEvent( KCal::Event * );

    void triggerReload();
    void cancelReload();

  signals:
    void dateTimesChanged( const QDateTime &, const QDateTime & );

  public slots:
    void slotInsertFreeBusy( KCal::FreeBusy *fb, const QString &email );

    void setDateTimes( const QDateTime &, const QDateTime & );

    void editFreeBusyUrl( KDGanttViewItem *item );

  protected slots:
    void slotUpdateGanttView( const QDateTime &, const QDateTime & );
    void slotScaleChanged( int );
    void slotCenterOnStart() ;
    void slotZoomToTime();
    void slotPickDate();

    void reload();

  protected:
    void timerEvent( QTimerEvent* );

  private:
    void updateFreeBusyData( FreeBusyItem * );

    bool findFreeSlot( KDateTime &dtFrom, KDateTime &dtTo );
    bool tryDate( KDateTime &tryFrom, KDateTime &tryTo );
    bool tryDate( FreeBusyItem *attendee,
                  KDateTime &tryFrom, KDateTime &tryTo );
    void updateStatusSummary();

    KDGanttView *mGanttView;
    QLabel *mStatusSummaryLabel;
    bool mIsOrganizer;
    QComboBox *scaleCombo;

    QDateTime mDtStart, mDtEnd;

    QTimer mReloadTimer;
};

#endif
