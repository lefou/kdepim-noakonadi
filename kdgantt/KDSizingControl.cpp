/****************************************************************************
** Copyright (C) 2001-2002 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KDGantt library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KDGantt licenses may use this file in
** accordance with the KDGantt Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/Public/products/ for
**   information about KDGantt Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
** As a special exception, permission is given to link this program
** with any edition of Qt, and distribute the resulting executable,
** without including the source code for Qt in the source distribution.
**
**********************************************************************/

#include "KDSizingControl.h"

/*!
  \class KDSizingControl KDSizingControl.h
  This class is a common-base class for all sizing controls in this
  library.

  It provides common signals and slots for minimizing and restoring
  child widgets.

  This class cannot be instantiated by itself, use one of the
  subclasses instead.
*/

/*!
  Constructs an empty KDSizing Control.

  \param parent the parent widget. This parameter is passed to the
  base class.
  \param name the internal widget name. This parameter is passed to
  the base class.

*/

KDSizingControl::KDSizingControl( QWidget* parent, const char* name, WFlags f )
    :QWidget( parent, name, f ), _isMinimized( false )
{
}


/*!
  Restores or minimizes the child widget. \a minimize() does exactly the
  opposite to this method.

  \param restore true to restore, false to minimize
  \sa minimize()
*/

void KDSizingControl::restore( bool restore )
{
    _isMinimized = !restore;
    if ( restore )
        emit restored( this );
    else
        emit minimized( this );
}




/*!
  Restores or minimizes the child widget. \a restore() does exactly the
  opposite to this method.

  \param minimize true to minimize, false to restore
  \sa restore()

*/

void KDSizingControl::minimize( bool minimize )
{
    _isMinimized = minimize;
    if ( minimize )
        emit minimized( this );
    else
        emit restored( this );
}


/*!
  Returns whether the widget is minimized.
*/

bool KDSizingControl::isMinimized() const
{
    return _isMinimized;
}

/*!
  Change state from either minimized to restored or visa versa.
*/

void KDSizingControl::changeState()
{
    restore(_isMinimized);
}


/*!
  \fn void KDSizingControl::minimized(  KDSizingControl* )

  This signal is emitted when the user hides a controlled widget. The
  KDSizingControl pointer given as parameter is a pointer to the widget
  itself. Normally the sender should not know the receiver, but in this
  case the receiver is likely the widget containing the KDSizingControl,
  and when the KDSizingControl widget is minimized/restored it might want
  to change stretching for the widget. See the example
  test/semisizingcontrol
*/


/*!
  \fn void KDSizingControl::restored(  KDSizingControl* )

  This signal is emitted when the user unhides a controlled widget. The
  KDSizingControl pointer given as parameter is a pointer to the widget
  itself. Normally the sender should not know the receiver, but in this
  case the receiver is likely the widget containing the KDSizingControl,
  and when the KDSizingControl widget is minimized/restored it might want
  to change stretching for the widget. See the example
  test/semisizingcontrol
*/
