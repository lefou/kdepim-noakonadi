/*
 *  birthdaydlg.cpp  -  dialog to pick birthdays from address book
 *  Program:  kalarm
 *  Copyright (c) 2002-2006 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"

#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kactioncollection.h>
#include <khbox.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kdebug.h>

#include "alarmcalendar.h"
#include "checkbox.h"
#include "colourcombo.h"
#include "editdlg.h"
#include "fontcolourbutton.h"
#include "kalarmapp.h"
#include "latecancel.h"
#include "preferences.h"
#include "reminder.h"
#include "repetition.h"
#include "shellprocess.h"
#include "soundpicker.h"
#include "specialactions.h"
#include "birthdaydlg.moc"

using namespace KCal;


class AddresseeItem : public Q3ListViewItem
{
	public:
		enum columns { NAME = 0, BIRTHDAY = 1 };
		AddresseeItem(Q3ListView* parent, const QString& name, const QDate& birthday);
		QDate birthday() const   { return mBirthday; }
		virtual QString key(int column, bool ascending) const;
	private:
		QDate     mBirthday;
		QString   mBirthdayOrder;
};


const KABC::AddressBook* BirthdayDlg::mAddressBook = 0;


BirthdayDlg::BirthdayDlg(QWidget* parent)
	: KDialog(parent),
	  mSpecialActionsButton(0)
{
	setCaption(i18n("Import Birthdays From KAddressBook"));
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	QWidget* topWidget = new QWidget(this);
        setMainWidget(topWidget);
	QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
	topLayout->setMargin(0);
	topLayout->setSpacing(spacingHint());

	// Prefix and suffix to the name in the alarm text
	// Get default prefix and suffix texts from config file
	KConfig* config = KGlobal::config();
	config->setGroup(QLatin1String("General"));
	mPrefixText = config->readEntry("BirthdayPrefix", i18n("Birthday: "));
	mSuffixText = config->readEntry("BirthdaySuffix");

	QGroupBox* textGroup = new QGroupBox(i18n("Alarm Text"), topWidget);
	topLayout->addWidget(textGroup);
	QGridLayout* grid = new QGridLayout(textGroup);
	grid->setMargin(marginHint());
	grid->setSpacing(spacingHint());
	QLabel* label = new QLabel(i18n("Pre&fix:"), textGroup);
	label->setFixedSize(label->sizeHint());
	grid->addWidget(label, 0, 0);
	mPrefix = new BLineEdit(mPrefixText, textGroup);
	mPrefix->setMinimumSize(mPrefix->sizeHint());
	label->setBuddy(mPrefix);
	connect(mPrefix, SIGNAL(focusLost()), SLOT(slotTextLostFocus()));
	mPrefix->setWhatsThis(i18n("Enter text to appear before the person's name in the alarm message, "
	                           "including any necessary trailing spaces."));
	grid->addWidget(mPrefix, 0, 1);

	label = new QLabel(i18n("S&uffix:"), textGroup);
	label->setFixedSize(label->sizeHint());
	grid->addWidget(label, 1, 0);
	mSuffix = new BLineEdit(mSuffixText, textGroup);
	mSuffix->setMinimumSize(mSuffix->sizeHint());
	label->setBuddy(mSuffix);
	connect(mSuffix, SIGNAL(focusLost()), SLOT(slotTextLostFocus()));
	mSuffix->setWhatsThis(i18n("Enter text to appear after the person's name in the alarm message, "
	                           "including any necessary leading spaces."));
	grid->addWidget(mSuffix, 1, 1);

	QGroupBox* group = new QGroupBox(i18n("Select Birthdays"), topWidget);
	topLayout->addWidget(group);
	QVBoxLayout* layout = new QVBoxLayout(group);
	layout->setMargin(0);
	mAddresseeList = new BListView(group);
	mAddresseeList->setMultiSelection(true);
	mAddresseeList->setSelectionMode(Q3ListView::Extended);
	mAddresseeList->setAllColumnsShowFocus(true);
	mAddresseeList->setFullWidth(true);
	mAddresseeList->addColumn(i18n("Name"));
	mAddresseeList->addColumn(i18n("Birthday"));
	connect(mAddresseeList, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	mAddresseeList->setWhatsThis(i18n("Select birthdays to set alarms for.\n"
	                                  "This list shows all birthdays in KAddressBook except those for which alarms already exist.\n\n"
	                                  "You can select multiple birthdays at one time by dragging the mouse over the list, "
	                                  "or by clicking the mouse while pressing Ctrl or Shift."));
	layout->addWidget(mAddresseeList);

	group = new QGroupBox(i18n("Alarm Configuration"), topWidget);
	topLayout->addWidget(group);
	QVBoxLayout* groupLayout = new QVBoxLayout(group);
	groupLayout->setMargin(marginHint());
	groupLayout->setSpacing(spacingHint());

	// Colour choice drop-down list
	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setMargin(0);
	hlayout->setSpacing(2*spacingHint());
	groupLayout->addLayout(hlayout);
	KHBox* box;
	mBgColourChoose = EditAlarmDlg::createBgColourChooser(&box, group);
	connect(mBgColourChoose, SIGNAL(highlighted(const QColor&)), SLOT(slotBgColourSelected(const QColor&)));
	hlayout->addWidget(box);
	hlayout->addStretch();

	// Font and colour choice drop-down list
	mFontColourButton = new FontColourButton(group);
	mFontColourButton->setFixedSize(mFontColourButton->sizeHint());
	connect(mFontColourButton, SIGNAL(selected()), SLOT(slotFontColourSelected()));
	hlayout->addWidget(mFontColourButton);

	// Sound checkbox and file selector
	mSoundPicker = new SoundPicker(group);
	mSoundPicker->setFixedSize(mSoundPicker->sizeHint());
	groupLayout->addWidget(mSoundPicker, 0, Qt::AlignLeft);

	// How much to advance warning to give
	mReminder = new Reminder(i18n("&Reminder"),
	                         i18n("Check to display a reminder in advance of the birthday."),
	                         i18n("Enter the number of days before each birthday to display a reminder. "
	                              "This is in addition to the alarm which is displayed on the birthday."),
	                         false, false, group);
	mReminder->setFixedSize(mReminder->sizeHint());
	mReminder->setMaximum(0, 364);
	mReminder->setMinutes(0, true);
	groupLayout->addWidget(mReminder, 0, Qt::AlignLeft);

	// Acknowledgement confirmation required - default = no confirmation
	hlayout = new QHBoxLayout();
	hlayout->setMargin(0);
	hlayout->setSpacing(2*spacingHint());
	groupLayout->addLayout(hlayout);
	mConfirmAck = EditAlarmDlg::createConfirmAckCheckbox(group);
	mConfirmAck->setFixedSize(mConfirmAck->sizeHint());
	hlayout->addWidget(mConfirmAck);
	hlayout->addSpacing(2*spacingHint());
	hlayout->addStretch();

	if (ShellProcess::authorised())    // don't display if shell commands not allowed (e.g. kiosk mode)
	{
		// Special actions button
		mSpecialActionsButton = new SpecialActionsButton(i18n("Special Actions..."), group);
		mSpecialActionsButton->setFixedSize(mSpecialActionsButton->sizeHint());
		hlayout->addWidget(mSpecialActionsButton);
	}

	// Late display checkbox - default = allow late display
	hlayout = new QHBoxLayout();
	hlayout->setMargin(0);
	hlayout->setSpacing(2*spacingHint());
	groupLayout->addLayout(hlayout);
	mLateCancel = new LateCancelSelector(false, group);
	mLateCancel->setFixedSize(mLateCancel->sizeHint());
	hlayout->addWidget(mLateCancel);
	hlayout->addStretch();

	// Simple repetition button
	mSimpleRepetition = new RepetitionButton(i18n("Simple Repetition"), false, group);
	mSimpleRepetition->setFixedSize(mSimpleRepetition->sizeHint());
	mSimpleRepetition->set(0, 0, true, 364*24*60);
	mSimpleRepetition->setWhatsThis(i18n("Set up an additional alarm repetition"));
	hlayout->addWidget(mSimpleRepetition);

	// Set the values to their defaults
	mFontColourButton->setDefaultFont();
	mFontColourButton->setBgColour(Preferences::defaultBgColour());
	mBgColourChoose->setColour(Preferences::defaultBgColour());     // set colour before setting alarm type buttons
	mLateCancel->setMinutes(Preferences::defaultLateCancel(), true, TimePeriod::DAYS);
	mConfirmAck->setChecked(Preferences::defaultConfirmAck());
	mSoundPicker->set(Preferences::defaultSound(), Preferences::defaultSoundType(), Preferences::defaultSoundFile(),
	                  Preferences::defaultSoundVolume(), -1, 0, Preferences::defaultSoundRepeat());
	if (mSpecialActionsButton)
		mSpecialActionsButton->setActions(Preferences::defaultPreAction(), Preferences::defaultPostAction());

	// Initialise the birthday selection list and disable the OK button
	loadAddressBook();
}

/******************************************************************************
* Load the address book in preparation for displaying the birthday selection list.
*/
void BirthdayDlg::loadAddressBook()
{
	if (!mAddressBook)
	{
        	mAddressBook = KABC::StdAddressBook::self(true);
		if (mAddressBook)
                	connect(mAddressBook, SIGNAL(addressBookChanged(AddressBook*)), SLOT(updateSelectionList()));
	}
	else
		updateSelectionList();
        if (!mAddressBook)
                KMessageBox::error(this, i18n("Error reading address book"));
}

/******************************************************************************
* Initialise or update the birthday selection list by fetching all birthdays
* from the address book and displaying those which do not already have alarms.
*/
void BirthdayDlg::updateSelectionList()
{
	// Compile a list of all pending alarm messages which look like birthdays
	QStringList messageList;
	KAEvent event;
	Event::List events = AlarmCalendar::resources()->events(KCalEvent::ACTIVE);
	for (Event::List::ConstIterator it = events.begin();  it != events.end();  ++it)
	{
		Event* kcalEvent = *it;
		event.set(kcalEvent);
		if (event.action() == KAEvent::MESSAGE
		&&  event.recurType() == KARecurrence::ANNUAL_DATE
		&&  (mPrefixText.isEmpty()  ||  event.message().startsWith(mPrefixText)))
			messageList.append(event.message());
	}

	// Fetch all birthdays from the address book
	for (KABC::AddressBook::ConstIterator abit = mAddressBook->begin();  abit != mAddressBook->end();  ++abit)
	{
		const KABC::Addressee& addressee = *abit;
		if (addressee.birthday().isValid())
		{
			// Create a list entry for this birthday
			QDate birthday = addressee.birthday().date();
			QString name = addressee.nickName();
			if (name.isEmpty())
				name = addressee.realName();
			// Check if the birthday already has an alarm
			QString text = mPrefixText + name + mSuffixText;
			bool alarmExists = messageList.contains(text);
			// Check if the birthday is already in the selection list
			bool inSelectionList = false;
			AddresseeItem* item = 0;
			for (Q3ListViewItem* qitem = mAddresseeList->firstChild();  qitem;  qitem = qitem->nextSibling())
			{
				item = dynamic_cast<AddresseeItem*>(qitem);
				if (item  &&  item->text(AddresseeItem::NAME) == name  &&  item->birthday() == birthday)
				{
					inSelectionList = true;
					break;
				}
			}

			if (alarmExists  &&  inSelectionList)
				delete item;     // alarm exists, so remove from selection list
			else if (!alarmExists  &&  !inSelectionList)
				new AddresseeItem(mAddresseeList, name, birthday);   // add to list
		}
	}
//	mAddresseeList->setUpdatesEnabled(true);

	// Enable/disable OK button according to whether anything is currently selected
	bool selection = false;
	for (Q3ListViewItem* item = mAddresseeList->firstChild();  item;  item = item->nextSibling())
		if (mAddresseeList->isSelected(item))
		{
			selection = true;
			break;
		}
	enableButtonOk(selection);
}

/******************************************************************************
* Return a list of events for birthdays chosen.
*/
QList<KAEvent> BirthdayDlg::events() const
{
	QList<KAEvent> list;
	QDate today = QDate::currentDate();
	KDateTime todayStart(today, KDateTime::ClockTime);
	int thisYear = today.year();
	int reminder = mReminder->minutes();

	for (Q3ListViewItem* item = mAddresseeList->firstChild();  item;  item = item->nextSibling())
	{
		if (mAddresseeList->isSelected(item))
		{
			AddresseeItem* aItem = dynamic_cast<AddresseeItem*>(item);
			if (aItem)
			{
				QDate date = aItem->birthday();
				date.setYMD(thisYear, date.month(), date.day());
				if (date <= today)
					date.setYMD(thisYear + 1, date.month(), date.day());
                                KDateTime dt( date, KDateTime::ClockTime );
				KAEvent event(dt,
				              mPrefix->text() + aItem->text(AddresseeItem::NAME) + mSuffix->text(),
				              mBgColourChoose->color(), mFontColourButton->fgColour(),
				              mFontColourButton->font(), KAEvent::MESSAGE, mLateCancel->minutes(),
				              mFlags);
				float fadeVolume;
				int   fadeSecs;
				float volume = mSoundPicker->volume(fadeVolume, fadeSecs);
				event.setAudioFile(mSoundPicker->file(), volume, fadeVolume, fadeSecs);
				QList<int> months;
				months.append(date.month());
				event.setRecurAnnualByDate(1, months, 0, Preferences::defaultFeb29Type(), -1, QDate());
				event.setNextOccurrence(todayStart, true);
#warning Check that birthdays do not trigger today if they land today
				event.setRepetition(mSimpleRepetition->interval(), mSimpleRepetition->count());
				if (reminder)
					event.setReminder(reminder, false);
				if (mSpecialActionsButton)
					event.setActions(mSpecialActionsButton->preAction(),
					                 mSpecialActionsButton->postAction());
				list.append(event);
			}
		}
	}
	return list;
}

/******************************************************************************
* Called when the OK button is selected to import the selected birthdays.
*/
void BirthdayDlg::slotOk()
{
	// Save prefix and suffix texts to use as future defaults
	KConfig* config = KGlobal::config();
	config->setGroup(QLatin1String("General"));
	config->writeEntry("BirthdayPrefix", mPrefix->text());
	config->writeEntry("BirthdaySuffix", mSuffix->text());
	config->sync();

	mFlags = (mSoundPicker->beep()             ? KAEvent::BEEP : 0)
	       | (mSoundPicker->repeat()           ? KAEvent::REPEAT_SOUND : 0)
	       | (mConfirmAck->isChecked()         ? KAEvent::CONFIRM_ACK : 0)
	       | (mFontColourButton->defaultFont() ? KAEvent::DEFAULT_FONT : 0)
	       |                                     KAEvent::ANY_TIME;
	KDialog::accept();
}

/******************************************************************************
* Called when the group of items selected changes.
* Enable/disable the OK button depending on whether anything is selected.
*/
void BirthdayDlg::slotSelectionChanged()
{
	for (Q3ListViewItem* item = mAddresseeList->firstChild();  item;  item = item->nextSibling())
		if (mAddresseeList->isSelected(item))
		{
			enableButtonOk(true);
			return;
		}
	enableButtonOk(false);

}

/******************************************************************************
* Called when the prefix or suffix text has lost keyboard focus.
* If the text has changed, re-evaluates the selection list according to the new
* birthday alarm text format.
*/
void BirthdayDlg::slotTextLostFocus()
{
	QString prefix = mPrefix->text();
	QString suffix = mSuffix->text();
	if (prefix != mPrefixText  ||  suffix != mSuffixText)
	{
		// Text has changed - re-evaluate the selection list
		mPrefixText = prefix;
		mSuffixText = suffix;
		loadAddressBook();
	}
}

/******************************************************************************
*  Called when the a new background colour has been selected using the colour
*  combo box.
*/
void BirthdayDlg::slotBgColourSelected(const QColor& colour)
{
	mFontColourButton->setBgColour(colour);
}

/******************************************************************************
*  Called when the a new font and colour have been selected using the font &
*  colour pushbutton.
*/
void BirthdayDlg::slotFontColourSelected()
{
	mBgColourChoose->setColour(mFontColourButton->bgColour());
}


/*=============================================================================
= Class: AddresseeItem
=============================================================================*/

AddresseeItem::AddresseeItem(Q3ListView* parent, const QString& name, const QDate& birthday)
	: Q3ListViewItem(parent),
	  mBirthday(birthday)
{
	setText(NAME, name);
	setText(BIRTHDAY, KGlobal::locale()->formatDate(mBirthday, true));
	mBirthdayOrder.sprintf("%04d%03d", mBirthday.year(), mBirthday.dayOfYear());
}

QString AddresseeItem::key(int column, bool) const
{
	if (column == BIRTHDAY)
		return mBirthdayOrder;
	return text(column).toLower();
}


/*=============================================================================
= Class: BListView
=============================================================================*/

BListView::BListView(QWidget* parent)
	: K3ListView(parent)
{
#warning Test select all/deselect
	KActionCollection* actcol = new KActionCollection(this);
	actcol->setAssociatedWidget(this);
	KStdAction::selectAll(actcol);
	KStdAction::deselect(this, SLOT(slotDeselect()), actcol);
}
