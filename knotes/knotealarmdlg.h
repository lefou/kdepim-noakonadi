/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 2005, Michael Brade <brade@kde.org>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
*******************************************************************/

#ifndef KNOTEALARMDLG_H
#define KNOTEALARMDLG_H

#include <kdialog.h>

class QButtonGroup;

namespace KCal {
class Journal;
}

namespace KPIM {
class KDateEdit;
class KTimeEdit;
}


class KNoteAlarmDlg
  : public KDialog
{
  Q_OBJECT
  public:
    KNoteAlarmDlg( const QString &caption, QWidget *parent = 0 );
    
    void setIncidence( KCal::Journal *journal );
    
  private slots:
    void slotOk();
    void slotButtonChanged( int );
    
  private:
    KCal::Journal *m_journal;
    
    KPIM::KDateEdit *m_atDate;
    KPIM::KTimeEdit *m_atTime, *m_inTime;
    QButtonGroup *m_buttons;
};

#endif
