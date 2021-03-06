/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signcommand.cpp

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
    you do not wish to do so, delete this exception statement from
    your version of the file, but you are not obligated to do so.  If
    your version.
*/

#include <config-kleopatra.h>

#include "signcommand.h"

#include <crypto/signemailcontroller.h>

#include <utils/exception.h>
#include <utils/input.h>
#include <utils/output.h>

#include <KLocale>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace boost;

class SignCommand::Private : public QObject {
    Q_OBJECT
private:
    friend class ::Kleo::SignCommand;
    SignCommand * const q;
public:
    explicit Private( SignCommand * qq )
        : q( qq ), controller()
    {

    }

private:
    void checkForErrors() const;

private Q_SLOTS:
    void slotSignersResolved();
    void slotMicAlgDetermined( const QString & );
    void slotDone();
    void slotError( int, const QString & );

private:
    shared_ptr<SignEMailController> controller;
};

SignCommand::SignCommand()
    : AssuanCommandMixin<SignCommand>(), d( new Private( this ) )
{

}

SignCommand::~SignCommand() {}

void SignCommand::Private::checkForErrors() const {

    if ( q->numFiles() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "SIGN is an email mode command, connection seems to be in filemanager mode" ) );

    if ( !q->recipients().empty() && !q->informativeRecipients() )
        throw Exception( makeError( GPG_ERR_CONFLICT ),
                         i18n( "RECIPIENT may not be given prior to SIGN, except with --info" ) );

    if ( q->inputs().empty() )
        throw Exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                         i18n( "At least one INPUT must be present" ) );

    if ( q->outputs().size() != q->inputs().size() )
        throw Exception( makeError( GPG_ERR_ASS_NO_INPUT ),
                         i18n( "INPUT/OUTPUT count mismatch" ) );

    if ( !q->messages().empty() )
        throw Exception( makeError( GPG_ERR_INV_VALUE ),
                         i18n( "MESSAGE command is not allowed before SIGN" ) );

}

int SignCommand::doStart() {

    d->checkForErrors();

    d->controller.reset( new SignEMailController( shared_from_this(), SignEMailController::GpgOLMode ) );
    d->controller->setProtocol( checkProtocol( EMail, AssuanCommand::AllowProtocolMissing ) );

    QObject::connect( d->controller.get(), SIGNAL(signersResolved()), d.get(), SLOT(slotSignersResolved() ) );
    QObject::connect( d->controller.get(), SIGNAL(reportMicAlg(QString)), d.get(), SLOT(slotMicAlgDetermined(QString)) );
    QObject::connect( d->controller.get(), SIGNAL(done()), d.get(), SLOT(slotDone()) );
    QObject::connect( d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)) );

    d->controller->startResolveSigners( senders() );

    return 0;
}

void SignCommand::Private::slotSignersResolved() {
    //hold local shared_ptr to member as q->done() deletes *this
    const shared_ptr<SignEMailController> cont( controller );

    try {
        const QString sessionTitle = q->sessionTitle();
        if ( !sessionTitle.isEmpty() )
            Q_FOREACH ( const shared_ptr<Input> & i, q->inputs() )
                i->setLabel( sessionTitle );

        controller->setDetachedSignature( q->hasOption("detached" ) );
        controller->setInputsAndOutputs( q->inputs(), q->outputs() );
        controller->start();

        return;

    } catch ( const Exception & e ) {
        q->done( e.error(), e.message() );
    } catch ( const std::exception & e ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unexpected exception in SignCommand::Private::slotRecipientsResolved: %1",
                      QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unknown exception in SignCommand::Private::slotRecipientsResolved") );
    }
    cont->cancel();
}

void SignCommand::Private::slotMicAlgDetermined( const QString & micalg ) {
    //hold local shared_ptr to member as q->done() deletes *this
    const shared_ptr<SignEMailController> cont( controller );

    try {

        q->sendStatus( "MICALG", micalg );
        return;

    } catch ( const Exception & e ) {
        q->done( e.error(), e.message() );
    } catch ( const std::exception & e ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unexpected exception in SignCommand::Private::slotMicAlgDetermined: %1",
                      QString::fromLocal8Bit( e.what() ) ) );
    } catch ( ... ) {
        q->done( makeError( GPG_ERR_UNEXPECTED ),
                 i18n("Caught unknown exception in SignCommand::Private::slotMicAlgDetermined") );
    }
    cont->cancel();
}

void SignCommand::Private::slotDone() {
    q->done();
}

void SignCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void SignCommand::doCanceled() {
    if ( d->controller )
        d->controller->cancel();
}

#include "signcommand.moc"
