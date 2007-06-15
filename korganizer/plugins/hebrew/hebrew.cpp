/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>

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
*/

#include "hebrew.h"
#include "configdialog.h"
#include "parsha.h"
#include "converter.h"
#include "holiday.h"

#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcalendarsystem.h>

bool Hebrew::IsraelP;

class HebrewFactory:public OldCalendarDecorationFactory
{
public:
  OldCalendarDecoration * create()
  {
    return new Hebrew;
  }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_hebrew, HebrewFactory )


QString Hebrew::shortText(const QDate & date) const
{

  KConfig config("korganizerrc", KConfig::NoGlobals );

  KConfigGroup group(&config,"Calendar/Hebrew Calendar Plugin");
  IsraelP =
    group.readEntry("Israel",
                         (KGlobal::locale()->country() == QLatin1String(".il")));
  Holiday::ParshaP = group.readEntry("Parsha", true);
  Holiday::CholP = group.readEntry("Chol_HaMoed", true);
  Holiday::OmerP = group.readEntry("Omer", true);
  QString label_text;

  int day = date.day();
  int month = date.month();
  int year = date.year();

  // core calculations!!
  struct DateResult result;

  Converter::SecularToHebrewConversion(year, month, day, /*0, */
                                       &result);
  int hebrew_day = result.day;
  int hebrew_month = result.month;
  int hebrew_year = result.year;
  int hebrew_day_of_week = result.day_of_week;
  bool hebrew_leap_year_p = result.hebrew_leap_year_p;
  int hebrew_kvia = result.kvia;
  int hebrew_day_number = result.hebrew_day_number;

  QStringList holidays =
    Holiday::FindHoliday(hebrew_month, hebrew_day,
                         hebrew_day_of_week + 1, hebrew_kvia,
                         hebrew_leap_year_p, IsraelP,
                         hebrew_day_number, hebrew_year);

  KCalendarSystem *cal = KCalendarSystem::create("hebrew");
  label_text = QString("%1 %2").arg(cal->dayString(date, false))
                                .arg(cal->monthName(date));

  foreach( QString holiday, holidays ) {
    label_text += '\n' + holiday;
  }

  return label_text;
}

QString Hebrew::info()
{
  return
    i18n("This plugin provides the date in the Jewish calendar.");
}

void Hebrew::configure(QWidget * parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);        //parent?

  dlg->exec();
  delete dlg;
}
