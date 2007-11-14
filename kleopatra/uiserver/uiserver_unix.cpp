/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/uiserver_unix.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "uiserver_p.h"

#include <QFile>
#include <QDir>

#include <stdexcept>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

using namespace Kleo;
using namespace boost;


static inline QString system_error_string() {
    return QString::fromLocal8Bit( strerror(errno) );
}

QString UiServer::Private::makeFileName( const QString & socket ) const {
    if ( !socket.isEmpty() )
        return socket;
    if ( tmpDir.status() != 0 )
        throw_<std::runtime_error>( tr( "Couldn't create directory %1: %2" ).arg( tmpDirPrefix() + "XXXXXXXX", system_error_string() ) );
    const QDir dir( tmpDir.name() );
    assert( dir.exists() );
    return dir.absoluteFilePath( "S.uiserver" );
}

void UiServer::Private::doMakeListeningSocket( const QByteArray & encodedFileName ) {
    // Create a Unix Domain Socket:
#ifdef HAVE_ASSUAN_SOCK_GET_NONCE
    const assuan_fd_t sock = assuan_sock_new( AF_UNIX, SOCK_STREAM, 0 );
#else
    const assuan_fd_t sock = ::socket( AF_UNIX, SOCK_STREAM, 0 );
#endif
    if ( sock == ASSUAN_INVALID_FD )
        throw_<std::runtime_error>( tr( "Couldn't create socket: %1" ).arg( system_error_string() ) );

    try {
        // Bind
        struct sockaddr_un sa;
        std::memset( &sa, 0, sizeof(sa) );
        sa.sun_family = AF_UNIX;
        std::strncpy( sa.sun_path, encodedFileName.constData(), sizeof( sa.sun_path ) - 1 );
#ifdef HAVE_ASSUAN_SOCK_GET_NONCE
        if ( assuan_sock_bind( sock, (struct sockaddr*)&sa, sizeof( sa ) ) )
#else
        if ( ::bind( sock, (struct sockaddr*)&sa, sizeof( sa ) ) )
#endif
            throw_<std::runtime_error>( tr( "Couldn't bind to socket: %1" ).arg( system_error_string() ) );

        // ### TODO: permissions?

#ifdef HAVE_ASSUAN_SOCK_GET_NONCE
        if ( assuan_sock_get_nonce( (struct sockaddr*)&sa, sizeof( sa ), &nonce ) )
            throw_<std::runtime_error>( tr("Couldn't get socket nonce: %1" ).arg( system_error_string() ) );
#endif

        // Listen
        if ( ::listen( sock, SOMAXCONN ) )
            throw_<std::runtime_error>( tr( "Couldn't listen to socket: %1" ).arg( system_error_string() ) );

        if ( !setSocketDescriptor( sock ) )
            throw_<std::runtime_error>( tr( "Couldn't pass socket to Qt: %1. This should not happen, please report this bug." ).arg( errorString() ) );

    } catch ( ... ) {
        ::close( sock );
        throw;
    }
}

