/*
    KOrganizer Alarm Daemon Client.

    This file is part of KOrganizer.

    Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "koalarmclient.h"

#include "alarmdockwindow.h"
#include "alarmdialog.h"

#include <kcal/calendarresources.h>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdatetime.h>
#include <kapplication.h>
#include <kwindowsystem.h>
#include <kconfiggroup.h>

#include <QPushButton>
#include <kglobal.h>
#include "korgacadaptor.h"
#include <QtDBus>

KOAlarmClient::KOAlarmClient( QObject *parent )
  : QObject( parent )
{
  new KOrgacAdaptor(this);
  QDBusConnection::sessionBus().registerObject("/ac", this);
  kDebug(5890) <<"KOAlarmClient::KOAlarmClient()";

  mDocker = new AlarmDockWindow;
  mDocker->show();
  connect( this, SIGNAL( reminderCount( int ) ), mDocker, SLOT( slotUpdate( int ) ) );
  connect( mDocker, SIGNAL( quitSignal() ), SLOT( slotQuit() ) );

  KConfig c( KStandardDirs::locate( "config", "korganizerrc" ) );
  KConfigGroup cg( &c, "Time & Date" );
  QString tz = cg.readEntry( "TimeZoneId" );
  kDebug(5890) <<"TimeZone:" << tz;

  mCalendar = new CalendarResources( tz );
  mCalendar->readConfig();
  mCalendar->load();

  connect( &mCheckTimer, SIGNAL( timeout() ), SLOT( checkAlarms() ) );

  KConfigGroup config(KGlobal::config(), "Alarms");
  int interval = config.readEntry( "Interval", 60 );
  kDebug(5890) <<"KOAlarmClient check interval:" << interval <<" seconds."
                << endl;
  mLastChecked = config.readEntry( "CalendarsLastChecked", QDateTime() );

  // load reminders that were active when quitting
  config.changeGroup( "General" );
  int numReminders = config.readEntry( "Reminders", 0 );
  for ( int i=1; i<=numReminders; ++i )
  {
    QString group( QString( "Incidence-%1" ).arg( i ) );
    config.changeGroup( group );
    QString uid = config.readEntry( "UID" );
    QDateTime dt = config.readEntry( "RemindAt", QDateTime() );
    if ( !uid.isEmpty() )
      createReminder( mCalendar->incidence( uid ), dt );
    config.deleteGroup();
  }
  config.changeGroup( "General" );
  if (numReminders) {
     config.writeEntry( "Reminders", 0 );
     config.sync();
  }

  checkAlarms();
  mCheckTimer.start( 1000 * interval );  // interval in seconds
}

KOAlarmClient::~KOAlarmClient()
{
  delete mCalendar;
  delete mDocker;
}

void KOAlarmClient::checkAlarms()
{
  KConfigGroup cfg( KGlobal::config(), "General" );

  if ( !cfg.readEntry( "Enabled", true ) ) return;

  QDateTime from = mLastChecked.addSecs( 1 );
  mLastChecked = QDateTime::currentDateTime();

  kDebug(5891) <<"Check:" << from.toString() <<" -" << mLastChecked.toString();

  QList<Alarm *> alarms = mCalendar->alarms( KDateTime( from, KDateTime::LocalZone ), KDateTime( mLastChecked, KDateTime::LocalZone ) );

  QList<Alarm *>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    kDebug(5891) <<"REMINDER:" << (*it)->parent()->summary();
    Incidence *incidence = mCalendar->incidence( (*it)->parent()->uid() );
    createReminder( incidence, QDateTime::currentDateTime() );
  }
}

void KOAlarmClient::createReminder( KCal::Incidence *incidence,
                                    const QDateTime &dt )
{
  if ( !incidence )
    return;

  AlarmDialog *dialog = new AlarmDialog();
  dialog->setIncidence( incidence );
  dialog->setRemindAt( dt );
  connect( dialog, SIGNAL( finishedSignal( AlarmDialog *) ), SLOT( slotRemove( AlarmDialog *) ) );
  connect( mDocker, SIGNAL( suspendAllSignal() ), dialog, SLOT( slotUser1() ) );
  connect( mDocker, SIGNAL( dismissAllSignal() ), dialog, SLOT( slotOk() ) );
  connect( this, SIGNAL( saveAllSignal() ), dialog, SLOT( slotSave() ) );
  dialog->wakeUp();
  mReminders.append( dialog );
  emit reminderCount( mReminders.count() );
  saveLastCheckTime();
}

void KOAlarmClient::slotQuit()
{
  emit saveAllSignal();
  saveLastCheckTime();
  quit();
}

void KOAlarmClient::saveLastCheckTime()
{
  KConfigGroup cg( KGlobal::config(), "Alarms");
  cg.writeEntry( "CalendarsLastChecked", mLastChecked );
  KGlobal::config()->sync();
}

void KOAlarmClient::quit()
{
  kDebug(5890) <<"KOAlarmClient::quit()";
  kapp->quit();
}

void KOAlarmClient::slotRemove( AlarmDialog *d )
{
  mReminders.removeAll( d );
  delete d;
  emit reminderCount( mReminders.count() );
}

bool KOAlarmClient::commitData( QSessionManager& )
{
  emit saveAllSignal();
  saveLastCheckTime();
  return true;
}

void KOAlarmClient::forceAlarmCheck()
{
  checkAlarms();
  saveLastCheckTime();
}

void KOAlarmClient::dumpDebug()
{
  KConfigGroup cfg( KGlobal::config(), "Alarms" );

  QDateTime lastChecked = cfg.readEntry( "CalendarsLastChecked", QDateTime() );

  kDebug(5890) <<"Last Check:" << lastChecked;
}

QStringList KOAlarmClient::dumpAlarms()
{
  KDateTime start = KDateTime( QDateTime::currentDateTime().date(),
                               QTime( 0, 0 ), KDateTime::LocalZone );
  KDateTime end = start.addDays( 1 ).addSecs( -1 );

  QStringList lst;
  // Don't translate, this is for debugging purposes.
  lst << QString("AlarmDeamon::dumpAlarms() from ") + start.toString()+ " to " +
         end.toString();

  QList<Alarm*> alarms = mCalendar->alarms( start, end );
  QList<Alarm*>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *a = *it;
    lst << QString("  ") + a->parent()->summary() + " ("
              + a->time().toString() + ')';
  }

  return lst;
}

void KOAlarmClient::debugShowDialog()
{
//   showAlarmDialog();
}

#include "koalarmclient.moc"
