/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <akonadi/entitytreemodel.h>
#include <akonadi/changerecorder.h>

#include "akonadi_next_export.h"

using namespace Akonadi;

class ContactsModelPrivate;

class AKONADI_NEXT_EXPORT ContactsModel : public EntityTreeModel
{
  Q_OBJECT
public:

  enum Roles
  {
    EmailCompletionRole = EntityTreeModel::UserRole
  };

  ContactsModel(Session *session, ChangeRecorder *monitor, QObject *parent = 0);
  virtual ~ContactsModel();

protected:
  virtual QVariant entityData(const Item &item, int column, int role=Qt::DisplayRole) const;

  virtual QVariant entityData(const Collection &collection, int column, int role=Qt::DisplayRole) const;

  virtual int columnCount(const QModelIndex &index = QModelIndex()) const;

  virtual QVariant entityHeaderData( int section, Qt::Orientation orientation, int role, HeaderGroup headerGroup ) const;

  /**
    Returns true if @p matchdata matches @p item using @p flags.
  */
  virtual bool entityMatch(const Akonadi::Item &item, const QVariant &matchData, Qt::MatchFlags flags ) const;

  /**
    Returns true if @p matchdata matches @p col using @p flags.
  */
  virtual bool entityMatch(const Akonadi::Collection &col, const QVariant &matchData, Qt::MatchFlags flags ) const;

private:
    Q_DECLARE_PRIVATE(ContactsModel)
    ContactsModelPrivate *d_ptr;
};

#endif
