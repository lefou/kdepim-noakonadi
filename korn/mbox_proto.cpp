/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "mbox_proto.h"

#include <kdebug.h>

#include <QList>
#include <QVector>
#include <QStringList>

#include "account_input.h"

void MBox_Protocol::configFillGroupBoxes( QStringList* groupBoxes ) const
{
	groupBoxes->append( "mbox" );
}

void MBox_Protocol::configFields( QVector< QWidget* >* vector, const QObject*, QList< AccountInput* > *result ) const
{
	result->append( new URLInput( vector->at( 0 ), i18n( "File:" ), "", "mailbox" ) );
}

void MBox_Protocol::readEntries( QMap< QString, QString >*, QMap< QString, QString >* ) const
{
}

void MBox_Protocol::writeEntries( QMap< QString, QString >* map ) const
{
	clearFields( map, (KIO_Protocol::Fields)( server | port | username | password | save_password | metadata ) );
}
