/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathMailboxIMAP4.h"
#endif

// Local includes
#include "EmpathMailboxIMAP4.h"

EmpathMailboxIMAP4::EmpathMailboxIMAP4(const QString & name)
    :   EmpathMailbox   (name),
        serverAddress_  (QString::null),
        serverPort_     (110),
        username_       (QString::null),
        password_       (QString::null)
{
    type_ = IMAP4;
    typeString_ = "IMAP4";
}


    void
EmpathMailboxIMAP4::s_checkMail()
{
    // STUB
}

    void
EmpathMailboxIMAP4::saveConfig()
{
    // STUB
}

    void
EmpathMailboxIMAP4::loadConfig()
{
    // STUB
}

// Set methods
        
    void
EmpathMailboxIMAP4::setServerAddress(const QString & serverAddress)
{
    serverAddress_    = serverAddress;
}

    void
EmpathMailboxIMAP4::setServerPort(Q_UINT32 serverPort)
{
    serverPort_ = serverPort;
}

    void
EmpathMailboxIMAP4::setUsername(const QString & username)
{
    username_ = username;
}

    void
EmpathMailboxIMAP4::setPassword(const QString & password)
{
    // STUB
    password_ = password;
}

// Get methods
        
    QString
EmpathMailboxIMAP4::serverAddress()
{
    return serverAddress_;
}

    Q_UINT32
EmpathMailboxIMAP4::serverPort()
{
    return serverPort_;
}

    QString
EmpathMailboxIMAP4::username()
{
    return username_;
}

    QString
EmpathMailboxIMAP4::password()
{
    // STUB
    return password_;
}

    bool
EmpathMailboxIMAP4::newMail() const
{
    // STUB
    return false;
}
    void
EmpathMailboxIMAP4::sync(const EmpathURL &)
{
    // STUB
}

    void
EmpathMailboxIMAP4::init()
{
    // STUB
    empathDebug("init() called");
}

// vim:ts=4:sw=4:tw=78
