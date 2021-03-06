/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef __MESSAGELIST_CORE_MODELINVARIANTINDEX_H__
#define __MESSAGELIST_CORE_MODELINVARIANTINDEX_H__

#include <qglobal.h> // defines uint, at least.

#include <messagelist/messagelist_export.h>

namespace MessageList
{

namespace Core
{

class ModelInvariantRowMapper;
class ModelInvariantRowMapperPrivate;
class RowShift;

/**
 * An invariant index that can be ALWAYS used to reference
 * an item inside a QAbstractItemModel.
 *
 * This class is meant to be used together with ModelInvariantRowMapper.
 */
class MESSAGELIST_EXPORT ModelInvariantIndex
{
  friend class ModelInvariantRowMapper;
  friend class ModelInvariantRowMapperPrivate;
  friend class RowShift;

public:
  ModelInvariantIndex();
  virtual ~ModelInvariantIndex();

public:
  /**
   * Returns true if this ModelInvariantIndex is valid, that is, it has been attacched
   * to a ModelInvariantRowMapper. Returns false otherwise.
   * An invalid index will always map to the current row -1 (which is invalid as QModelIndex row).
   */
  bool isValid() const;

  /**
   * Returns the current model index row for this invariant index. This function
   * calls the mapper and asks it to perform the persistent mapping.
   * If this index isn't valid then the returned value is -1.
   *
   * If you actually own the row mapper then you may save some clock cycles
   * by calling the modelInvariantIndexToModelIndexRow() by your own. If you don't
   * own the mapper then this function is the only way to go.
   */
  int currentModelIndexRow();

private:
  class Private;
  Private * const d;
};

} // namespace Core

} // namespace MessageList


#endif //!__MESSAGELIST_CORE_MODELINVARIANTINDEX_H__
