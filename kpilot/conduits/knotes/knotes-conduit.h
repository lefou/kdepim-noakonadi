// knotes-conduit.h
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$


#ifndef _KNOTES_CONDUIT_H
#define _KNOTES_CONDUIT_H

#include "baseConduit.h"

class PilotRecord;
class PilotMemo;

#include <qstring.h>
#include <qmap.h>


// This class stores information about notes.
// We construct a map that associates note names
// with NotesSettings so that we can quickly 
// find out which notes are new / changed / old
// or deleted without having to re-read all those
// KNotes config files every time.
//
//
class NotesSettings
{
public:
	// Both of these constructores are deprecated.
	//
	//
	NotesSettings() {} ;
	NotesSettings(const QString& name,
		const QString& configPath,
		const QString& dataPath,
		unsigned long pilotID) :
		nN(name),
		cP(configPath),dP(dataPath),id(pilotID),
		checksum(0L),csValid(false) {} ;

	NotesSettings(const QString &configPath,
	        const QString &notesdir,
		KConfig& c);

	const QString& name() const { return nN; } ;
	const QString& configPath() const { return cP; } ;
	const QString& dataPath() const { return dP; } ;
	unsigned long pilotID() const { return id; } ;

	bool isNew() const { return id == 0L ; } ;
	void setId(unsigned long i) { id=i; } ;

	QString checkSum() const { return checksum; } ;
	bool isCheckSumValid() const { return csValid; } ;
	/**
	* Is the file changed? If we have no valid checksum,
	* then it must be changed. Otherwise, recompute the
	* checksum and if the new checksum is different from
	* the old one, then the note has changed.
	*/
	bool isChanged() const 
	{ 
		QString newCS = computeCheckSum();
		return !csValid || (newCS != checksum) ; 
	}

	QString updateCheckSum() 
	{ 
		checksum=computeCheckSum(); 
		csValid = !checksum.isNull();
		return checksum; 
	} ;
	QString computeCheckSum() const;

	/**
	* Reads a note file into memory. This reads at most
	* PilotMemo::MAX_MEMO_LEN bytes into the buffer text,
	* which must be at least PilotMemo::MAX_MEMO_LEN+1
	* bytes large (1 extra byte to accommodate trailing 0).
	*/
	int readNotesData(char *text);

private:
	QString nN,cP,dP;
	unsigned long id;
	/**
	* Since KNotes (re)writes the notes files all the time,
	* we need a different way of detecting when a note has
	* changed. We don't want to keep copying all the notes
	* to the Pilot if they haven't really changed, so we
	* add a checksum when the note is copied to the Pilot.
	* The checksum is just the (16-byte) md5 digest of the note.
	*/
	QString checksum;
	bool csValid;		// Checksum valid?
} ;

typedef QMap<QString,NotesSettings> NotesMap;

class KNotesConduit : public BaseConduit
{
public:
  KNotesConduit(eConduitMode mode);
  virtual ~KNotesConduit();
  
  virtual void doSync();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() ; // { return NULL; }
  virtual void doTest();

protected:
	int notesToPilot(NotesMap&);
	int pilotToNotes(NotesMap&);

	/**
	* KNotes can be new or changed relative to the Pilot.
	* Each requires special handling.
	*/
	bool addNewNote(NotesSettings&);
	bool changeNote(NotesSettings&);

	bool changeMemo(NotesMap&,NotesMap::Iterator,PilotMemo *);
	bool newMemo(NotesMap&,unsigned long id,PilotMemo *);
	/**
	* Delete a KNote associated with a particular
	* Pilot memo. The note to be deleted is the
	* one the iterator points to. The note is also
	* removed from the map.
	*/
	bool deleteNote(NotesMap&,NotesMap::Iterator *,unsigned long);

private:
	/**
	* Read the global KPilot config file for settings
	* particular to the KNotes conduit.
	* @ref fDeleteNoteForMemo
	*/
	void readConfig();

	/**
	* This reflects the setting in the KNotes conduit
	* setup whether or not to delete the KNote associated
	* with a particular Pilot memo when the memo itself
	* is deleted.
	*/
	bool fDeleteNoteForMemo;
};

// $Log$
// Revision 1.4  2000/12/05 07:44:01  adridg
// Cleanup
//
// Revision 1.3  2000/11/26 01:43:21  adridg
// Two-way syncs
//
// Revision 1.2  2000/11/24 17:54:28  adridg
// Two-way sync
//
// Revision 1.1  2000/11/20 00:22:28  adridg
// New KNotes conduit
//
#endif
