/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_BLOGMODEL_H
#define AKONADI_BLOGMODEL_H

#include <akonadi/itemmodel.h>
#include <akonadi/job.h>

/**
  A flat self-updating message model.
*/
class BlogModel : public Akonadi::ItemModel
{
    Q_OBJECT

public:
    /**
      Column types.
    */
    enum Column {
        Date = Qt::UserRole, /**< Date column. */
        User, /**< Usre column. */
        Text, /**< Textr column. */
        Picture /**< url to a profile picture column. */
    };

    /**
      Creates a new message model.

      @param parent The parent object.
    */
    explicit BlogModel( QObject* parent = 0 );

    /**
      Deletes the message model.
    */
    virtual ~BlogModel();

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

private:
    class Private;
    Private* const d;
};

#endif
