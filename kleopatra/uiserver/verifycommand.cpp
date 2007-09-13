/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/verifycommand.cpp

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

#include "verifycommand.h"

#include <QFile>
#include <QObject>
#include <QIODevice>
#include <QMessageBox>

#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgme++/error.h>
#include <gpgme++/verificationresult.h>

#include <gpg-error.h>

#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using namespace Kleo;
namespace {

    struct VerifyException : public std::exception {
        VerifyException( const char* _text )
        :text(_text)
        {}
        virtual ~VerifyException() throw() {}
        const QByteArray text;
    };
}

class VerificationResultDialog : public QDialog
{
    Q_OBJECT
public:
    VerificationResultDialog( const GpgME::VerificationResult& result )
    {

    }
    virtual ~VerificationResultDialog() {}
};


class VerifyCommand::Private : public QObject
{
    Q_OBJECT
public:
    Private( VerifyCommand * qq )
        :q( qq ), backend(0), showDetails(false), dialog(0)
    {}
    ~Private()
    {
        delete dialog;
    }

    VerifyCommand *q;
    const CryptoBackend::Protocol *backend;
    bool showDetails;
    VerificationResultDialog * dialog;

    void findCryptoBackend();

    struct Input
    {
        Input() : type( Detached ), message( 0 ), signature( 0 ) {}
        enum SignatureType {
            Detached=0,
            Opaque
        };

        SignatureType type;
        QIODevice* message;
        QString messageFileName;
        QIODevice* signature;
    };

    QList<Input> inputList;
    QList<Input> setupInput( GpgME::Error& error, QString& errorDetails ) const;

public Q_SLOTS:
    void slotVerifyOpaqueResult(const GpgME::VerificationResult &, const QByteArray &);
    void slotVerifyDetachedResult(const GpgME::VerificationResult &);
    void slotProgress( const QString& what, int current, int total );
    void parseCommandLine( const std::string & line );
private Q_SLOTS:
    void slotDialogClosed();
private:
    void sendBriefResult( const GpgME::VerificationResult & result ) const;
    QString processSignature( const GpgME::Signature& sig ) const;
    void showVerificationResultDialog( const GpgME::VerificationResult& result );
};

VerifyCommand::VerifyCommand()
    : AssuanCommandMixin<VerifyCommand>(),
      d( new Private( this ) )
{
}

VerifyCommand::~VerifyCommand() {}

void VerifyCommand::Private::findCryptoBackend()
{
    // FIXME this could be either SMIME or OpenPGP, find out from headers
    const bool isSMIME = false;
    if ( isSMIME )
        backend = Kleo::CryptoBackendFactory::instance()->smime();
    else
        backend = Kleo::CryptoBackendFactory::instance()->openpgp();
}

QList<VerifyCommand::Private::Input> VerifyCommand::Private::setupInput( GpgME::Error& error, QString& errorDetails ) const
{
    error = GpgME::Error();
    errorDetails = QString();

    const int numSignatures = q->numBulkInputDevices( "INPUT" );
    const int numMessages = q->numBulkInputDevices( "MESSAGE" );

    if ( numSignatures == 0 )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT );
        errorDetails = "At least one signature must be provided";
        return QList<Input>();
    }

    if ( numMessages > 0 && numMessages != numSignatures )
    {
        error = GpgME::Error( GPG_ERR_ASS_NO_INPUT ); //TODO use better error code if possible
        errorDetails = "The number of MESSAGE inputs must be either equal to the number of signatures or zero";
        return QList<Input>();
    }

    QList<Input> inputs;

    if ( numMessages == numSignatures )
    {
        for ( int i = 0; i < numSignatures; ++i )
        {
            Input input;
            input.type = Input::Detached;
            input.signature = q->bulkInputDevice( "INPUT", i );
            input.message = q->bulkInputDevice( "MESSAGE", i );
            assert( input.signature );
            assert( input.message );
            inputs.append( input );
        }
        return inputs;
    }

    assert( numMessages == 0 );
    
    for ( int i = 0; i < numSignatures; ++i )
    {
        Input input;
        input.signature = q->bulkInputDevice( "INPUT", i );
        assert( input.signature );
        const QString fname = q->bulkInputDeviceFileName( "INPUT", i );
        if ( !fname.isEmpty() && fname.endsWith( ".sig", Qt::CaseInsensitive )
                || fname.endsWith( ".asc", Qt::CaseInsensitive ) )
        { //detached signature file
            const QString msgFileName = fname.left( fname.length() - 4 );
            // TODO: handle error if msg file does not exist
            input.type = Input::Detached;
            input.messageFileName = msgFileName;
        }
        else // opaque
        {
            input.type = Input::Opaque;
        }
        inputs.append( input );
    }
    return inputs;
}

static QString summaryToString( const GpgME::Signature::Summary summary )
{
    QString result;
    if ( summary & GpgME::Signature::Green )
        result = "GREEN ";
    return result;
}

QString VerifyCommand::Private::processSignature( const GpgME::Signature& sig ) const
{
    // FIXME review, should we continue, here?
    if ( sig.isNull() ) {
        q->done( makeError(GPG_ERR_GENERAL) );
        return QString();
    }
    return summaryToString( sig.summary() ) + sig.fingerprint();
}

void VerifyCommand::Private::sendBriefResult( const GpgME::VerificationResult & result ) const
{
    if ( result.isNull() ) {
        q->done( makeError(GPG_ERR_GENERAL) );
        return;
    }
    if ( result.error() ) {
        q->done( result.error() );
        return;
    }

    std::vector<GpgME::Signature> sigs = result.signatures();
    if ( sigs.size() == 0 ) {
        q->done( makeError(GPG_ERR_GENERAL) );
        return;
    }

    QStringList resultStrings;
    Q_FOREACH( GpgME::Signature sig, sigs )
        resultStrings.append( processSignature( sig ) );

    if ( const int err = q->sendStatus( "VERIFY", resultStrings.join("\n") ) )
        q->done( err );
}

void VerifyCommand::Private::slotDialogClosed()
{
    assert(dialog);
    delete dialog;
    dialog = 0;
    q->done();
}

void VerifyCommand::Private::showVerificationResultDialog( const GpgME::VerificationResult& result )
{
    dialog = new VerificationResultDialog( result );
    connect( dialog, SIGNAL( accepted() ), this, SLOT( slotDialogClosed() ) );
    connect( dialog, SIGNAL( rejected() ), this, SLOT( slotDialogClosed() ) );
    dialog->show();
}

void VerifyCommand::Private::slotVerifyOpaqueResult( const GpgME::VerificationResult & result ,
                                                     const QByteArray & stuff )
{
    Q_UNUSED( stuff )
    sendBriefResult( result );
    if ( showDetails )
        showVerificationResultDialog( result );
    else
        q->done();
}

void VerifyCommand::Private::slotVerifyDetachedResult( const GpgME::VerificationResult & result )
{
    sendBriefResult( result );
    if ( showDetails )
        showVerificationResultDialog( result );
    else
        q->done();
}

void VerifyCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void VerifyCommand::Private::parseCommandLine( const std::string & line )
{

}

int VerifyCommand::doStart()
{
    d->parseCommandLine("");
    d->showDetails = !hasOption("silent");

    {
        GpgME::Error error;
        QString details;
        d->inputList = d->setupInput( error, details );
        if ( error )
        {
            done( error, details );
            return error;
        }
    }

    // FIXME check options

    d->findCryptoBackend(); // decide on smime or openpgp
    assert(d->backend);

    assert( !d->inputList.isEmpty() );

    const Private::Input input = d->inputList.first();

    if ( input.type == Private::Input::Opaque )
    {
        //fire off appropriate kleo verification job
        VerifyOpaqueJob * const job = d->backend->verifyOpaqueJob();
        assert(job);
        QObject::connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
                        d.get(),  SLOT(slotVerifyOpaqueResult(GpgME::VerificationResult,QByteArray)) );
        QObject::connect( job, SIGNAL(progress(QString,int,int)),
                        d.get(), SLOT(slotProgress(QString,int,int)) );

        //FIXME: readAll() save enough?
        const GpgME::Error error = job->start( input.signature->readAll() );
        if ( error ) 
            done( error );
        return error;
    }
    else
    {
        //fire off appropriate kleo verification job
        VerifyDetachedJob * const job = d->backend->verifyDetachedJob();
        assert(job);
        QObject::connect( job, SIGNAL(result(GpgME::VerificationResult)),
                        d.get(),  SLOT(slotVerifyDetachedResult(GpgME::VerificationResult)) );
        QObject::connect( job, SIGNAL(progress(QString,int,int)),
                        d.get(), SLOT(slotProgress(QString,int,int)) );


        //FIXME: readAll save enough
        const QByteArray signature = input.signature->readAll();
        QByteArray message;
        if ( input.message )
        {
            message = input.message->readAll();
        }
        else
        {
            QFile file( input.messageFileName );
            if ( !file.open( QIODevice::ReadOnly ) )
            {
                done( GPG_ERR_ASS_NO_INPUT, QString( "Couldn't read file %1" ).arg( input.messageFileName ) ); // FIXME: use better error code 
                return GPG_ERR_ASS_NO_INPUT;
            }
            message = file.readAll();
            file.close();
        }
        const GpgME::Error error = job->start( signature, message );
        if ( error )
            done( error );
        return error;
    }

    assert( false );
    return 0; // never reached
}

void VerifyCommand::doCanceled()
{
    if ( d->dialog ) {
        delete d->dialog;
        d->dialog = 0;
    }
}


#include "verifycommand.moc"
