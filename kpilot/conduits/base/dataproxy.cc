/* dataproxy.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "dataproxy.h"
#include "record.h"

#include "options.h"

DataProxy::DataProxy() : fIterator( fRecords )
{
	FUNCTIONSETUP;
	fLastId = 0;
}

DataProxy::~DataProxy()
{
	FUNCTIONSETUP;
}

QString DataProxy::create( Record *record )
{
	FUNCTIONSETUP;
	
	// Temporary id.
	fLastId = fLastId--;
	QString recordId = QString::number( fLastId );
	
	// Make sure that the new record has the right id and add the record.
	record->setId( recordId );
	fRecords.insert( recordId, record );
	
	// Update rollback/volatility information.
	fCreated.insert( recordId, false );
	fCounter.created();
	
	return recordId;
}

void DataProxy::remove( const QString &id )
{
	FUNCTIONSETUP;
	
	Record *rec = fRecords.value( id );
	if( rec == 0L )
	{
		// No record
		return;
	}
	
	// Remove record.
	fRecords.remove( id );
	
	// Update rollback/volatility information.
	fDeletedRecords.insert( rec->id(), rec );
	fDeleted.insert( rec->id(), false );
	fCounter.deleted();
}

void DataProxy::update( const QString &id, Record *newRecord )
{
	FUNCTIONSETUP;
	
	Record *oldRecord = fRecords.value( id );
	if( oldRecord == 0L )
	{
		// No record, should not happen.
		DEBUGKPILOT << fname << ": There is no record with id " << id
			<< ". Record not updated and not added" << endl;
		return;
	}
	
	// Make sure that the new record has the right id and update the old record.
	newRecord->setId( id );
	fRecords.insert( id, newRecord );
	
	// Update rollback/volatility information.
	fOldRecords.insert( id, oldRecord->duplicate() );
	fCounter.updated();
}

QList<QString> DataProxy::ids() const
{
	return fRecords.keys();
}

const CUDCounter* DataProxy::counter() const
{
	FUNCTIONSETUP;
	
	return &fCounter;
}

void DataProxy::syncFinished()
{
	FUNCTIONSETUP;
	
	fCounter.setEndCount( fRecords.size() );
}

void DataProxy::setIterateMode( const Mode m )
{
	FUNCTIONSETUP;
	
	fMode = m;
}

unsigned int DataProxy::recordCount() const
{
	return fRecords.size();
}

Record* DataProxy::find( const QString &id ) const
{
	FUNCTIONSETUP;
	return fRecords.value( id );
}

void DataProxy::resetIterator()
{
	fIterator = QMapIterator<QString, Record*>( fRecords );
}

bool DataProxy::hasNext() const
{
	FUNCTIONSETUP;
	
	if( fMode == All )
	{
		return fIterator.hasNext();
	}
	else
	{
		QMapIterator<QString, Record*> tmpIt = fIterator;
		while( tmpIt.hasNext() )
		{
			Record *rec = tmpIt.next().value();
			if( rec->isModified() )
			{
				return true;
			}
		}
	}
	
	return false;
}

Record* DataProxy::next()
{
	FUNCTIONSETUP;
	
	if( fMode == All )
	{
			return fIterator.next().value();
	}
	else
	{
		while( fIterator.hasNext() )
		{
			Record *rec = fIterator.next().value();
			if( rec->isModified() )
			{
				return rec;
			}
		}
	}
	
	return 0L;
}

bool DataProxy::commit()
{
	FUNCTIONSETUP;
	
	// Commit created records.
	QStringListIterator it( fCreated.keys() );
	
	while( it.hasNext() )
	{
		QString id = it.next();
		
		Record *rec = find( id );
		if( rec && !fCreated.value( id ) )
		{
			commitCreate( rec );
			
			// Put the record with the new id in.
			if( rec->id() != id )
			{
				fCreated.remove( id );
				fCreated.insert( rec->id(), true );
			
				fRecords.remove( id );
				fRecords.insert( rec->id(), rec );
			
				// TODO: store the (rec->id(), id) pair somewhere to change the mapping.
			}
			else
			{
				fCreated.insert( rec->id(), true );
			}
		}
	}
	
	// Commit updated records.
	QListIterator<Record*> i( fOldRecords.values() );
	while( i.hasNext() )
	{
		// i.next() contains the old values.
		Record *oldRec = i.next();
		QString id = oldRec->id();
		
		// Look up the new values
		Record *rec = find( id );
		
		if( rec && !fCreated.value( id ) )
		{
			commitUpdate( rec );
			QString newId = rec->id();
			
			if( newId != id )
			{
				oldRec->setId( newId );
				// TODO: store the (newId, id) pair somewhere to change the mapping.
			}
			fUpdated.insert( rec->id(), true );
		}
	}
	
	// Commit deleted records
	i = QListIterator<Record*>( fDeletedRecords.values() );
	while( i.hasNext() )
	{
		Record *oldRec = i.next();
		
		if( !fDeleted.value( oldRec->id() ) )
		{
			DEBUGKPILOT << fname << ": deleting record " << oldRec->id() << endl;
			commitDelete( oldRec );
			fDeleted.insert( oldRec->id(), true );
		}
	}
	
	return true;
}

bool DataProxy::rollback()
{
	FUNCTIONSETUP;
	
	// Delete committed new records.
	QStringListIterator it( fCreated.keys() );
	
	while( it.hasNext() )
	{
		QString id = it.next();
		
		// Only undo creates that are committed.
		Record *rec = find( id );
		if( rec && fCreated.value( id ) )
		{
			DEBUGKPILOT << fname << ": deleting created record " << rec->id() << endl;
			
			commitDelete( rec );
			fCreated.insert( rec->id(), false );
		}
	}
	
	// Undo changes to updated records.
	QListIterator<Record*> i( fOldRecords.values() );
	while( i.hasNext() )
	{
		Record *oldRec = i.next();
		if( fUpdated.value( oldRec->id() ) )
		{
			DEBUGKPILOT << fname << ": restoring changed record " << oldRec->id() << endl;
			
			QString oldId = oldRec->id();
			commitUpdate( oldRec );
			
			// Id might have changed
			if( oldRec->id() != oldId )
			{
				fUpdated.remove( oldId );	
			}
			
			fUpdated.insert( oldRec->id(), false );
		}
	}
	
	// Restore deleted records.
	i = QListIterator<Record*>( fDeletedRecords.values() );
	while( i.hasNext() )
	{
		Record *oldRec = i.next();
		
		if( fDeleted.value( oldRec->id() ) )
		{
			DEBUGKPILOT << fname << ": restoring deleted record " << oldRec->id() << endl;
		
			QString oldId = oldRec->id();
			commitCreate( oldRec );
			
			// Id might have changed
			if( oldRec->id() != oldId )
			{
				fDeleted.remove( oldId );	
			}
			
			fDeleted.insert( oldRec->id(), false );
		}
	}
	
	return true;
}
