// kmmsginfo.cpp


#include "kmmsginfo.h"
#include "kmmessage.h"
#include "kmmessagetag.h"
#include "stringutil.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <mimelib/datetime.h>
#include <QByteArray>

using namespace KMail;

/** The old status format, only one at a time possible. Needed
    for upgrade path purposes. */
typedef enum
{
    KMLegacyMsgStatusUnknown=' ',
    KMLegacyMsgStatusNew='N',
    KMLegacyMsgStatusUnread='U',
    KMLegacyMsgStatusRead='R',
    KMLegacyMsgStatusOld='O',
    KMLegacyMsgStatusDeleted='D',
    KMLegacyMsgStatusReplied='A',
    KMLegacyMsgStatusForwarded='F',
    KMLegacyMsgStatusQueued='Q',
    KMLegacyMsgStatusSent='S',
    KMLegacyMsgStatusFlag='G'
} KMLegacyMsgStatus;


class KMMsgInfo::KMMsgInfoPrivate
{
public:
    enum {
        SUBJECT_SET = 0x01, TO_SET = 0x02, REPLYTO_SET = 0x04, MSGID_SET=0x08,
        DATE_SET = 0x10, OFFSET_SET = 0x20, SIZE_SET = 0x40, SIZESERVER_SET = 0x80,
        XMARK_SET=0x100, FROM_SET=0x200, FILE_SET=0x400, ENCRYPTION_SET=0x800,
        SIGNATURE_SET=0x1000, MDN_SET=0x2000, REPLYTOAUX_SET = 0x4000,
        STRIPPEDSUBJECT_SET = 0x8000,  UID_SET = 0x10000,

        ALL_SET = 0xFFFFFF, NONE_SET = 0x000000
    };
    uint modifiers;
    QString subject, from, to, replyToIdMD5, replyToAuxIdMD5,
            strippedSubjectMD5, msgIdMD5, xmark, file;
    off_t folderOffset;
    size_t msgSize, msgSizeServer;
    time_t date;
    KMMsgEncryptionState encryptionState;
    KMMsgSignatureState signatureState;
    KMMsgMDNSentState mdnSentState;
    ulong UID;

    KMMsgInfoPrivate() : modifiers(NONE_SET) { }
    KMMsgInfoPrivate& operator=(const KMMsgInfoPrivate& other) {
        modifiers = NONE_SET;
        if (other.modifiers & SUBJECT_SET) {
            modifiers |= SUBJECT_SET;
            subject = other.subject;
        }
        if (other.modifiers & STRIPPEDSUBJECT_SET) {
            modifiers |= STRIPPEDSUBJECT_SET;
            strippedSubjectMD5 = other.strippedSubjectMD5;
        }
        if (other.modifiers & FROM_SET) {
            modifiers |= FROM_SET;
            from = other.from;
        }
        if (other.modifiers & FILE_SET) {
            modifiers |= FILE_SET;
            file = other.from;
        }
        if (other.modifiers & TO_SET) {
            modifiers |= TO_SET;
            to = other.to;
        }
        if (other.modifiers & REPLYTO_SET) {
            modifiers |= REPLYTO_SET;
            replyToIdMD5 = other.replyToIdMD5;
        }
        if (other.modifiers & REPLYTOAUX_SET) {
            modifiers |= REPLYTOAUX_SET;
            replyToAuxIdMD5 = other.replyToAuxIdMD5;
        }

        if(other.modifiers & MSGID_SET) {
            modifiers |= MSGID_SET;
            msgIdMD5 = other.msgIdMD5;
        }
        if(other.modifiers & XMARK_SET) {
            modifiers |= XMARK_SET;
            xmark = other.xmark;
        }
        if(other.modifiers & OFFSET_SET) {
            modifiers |= OFFSET_SET;
            folderOffset = other.folderOffset;
        }
        if(other.modifiers & SIZE_SET) {
            modifiers |= SIZE_SET;
            msgSize = other.msgSize;
        }
        if(other.modifiers & DATE_SET) {
            modifiers |= DATE_SET;
            date = other.date;
        }
        if(other.modifiers & ENCRYPTION_SET) {
            modifiers |= ENCRYPTION_SET;
            encryptionState = other.encryptionState;
        }
        if(other.modifiers & SIGNATURE_SET) {
            modifiers |= SIGNATURE_SET;
            signatureState = other.signatureState;
        }
        if(other.modifiers & MDN_SET) {
            modifiers |= MDN_SET;
            mdnSentState = other.mdnSentState;
        }
        if(other.modifiers & SIZESERVER_SET) {
            modifiers |= SIZESERVER_SET;
            msgSizeServer = other.msgSizeServer;
        }
        if(other.modifiers & UID_SET) {
            modifiers |= UID_SET;
            UID = other.UID;
        }
        return *this;
    }
};

//-----------------------------------------------------------------------------
#ifdef KMAIL_SQLITE_INDEX
KMMsgInfo::KMMsgInfo(KMFolder* p, char* data, short len, sqlite_int64 dbId) :
    KMMsgBase(p, data, len, dbId),
#else
KMMsgInfo::KMMsgInfo(KMFolder* p, off_t off, short len) :
    KMMsgBase(p, off, len),
#endif
    kd(0)
{
    setEnableUndo(true);
}


//-----------------------------------------------------------------------------
KMMsgInfo::~KMMsgInfo()
{
    delete kd;
}


#if 0 // currently unused
//-----------------------------------------------------------------------------
KMMsgInfo& KMMsgInfo::operator=(const KMMsgInfo& other)
{
    KMMsgBase::assign(&other);
    if(other.kd) {
        if(!kd)
            kd = new KMMsgInfoPrivate;
        *kd = *other.kd;
    } else {
        delete kd;
        kd = 0;
    }
    mStatus = other.status();
    return *this;
}
#endif

//-----------------------------------------------------------------------------
KMMsgInfo& KMMsgInfo::operator=(const KMMessage& msg)
{
    KMMsgBase::assign(&msg.toMsgBase());
    if(!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers = KMMsgInfoPrivate::ALL_SET;
    kd->subject = msg.subject();
    kd->from = msg.fromStrip();
    kd->to = msg.toStrip();
    kd->replyToIdMD5 = msg.replyToIdMD5();
    kd->replyToAuxIdMD5 = msg.replyToAuxIdMD5();
    kd->strippedSubjectMD5 = msg.strippedSubjectMD5();
    kd->msgIdMD5 = msg.msgIdMD5();
    kd->xmark = msg.xmark();
    mStatus = msg.messageStatus();
    kd->folderOffset = msg.folderOffset();
    kd->msgSize = msg.msgSize();
    kd->date = msg.date();
    kd->file = msg.fileName();
    kd->encryptionState = msg.encryptionState();
    kd->signatureState = msg.signatureState();
    kd->mdnSentState = msg.mdnSentState();
    kd->msgSizeServer = msg.msgSizeServer();
    kd->UID = msg.UID();
    if ( msg.tagList() ) {
      if ( !mTagList )
        mTagList = new KMMessageTagList();
      *mTagList = *msg.tagList();
    } else {
      delete mTagList;
      mTagList = 0;
    }
    return *this;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::init(const QByteArray& aSubject, const QByteArray& aFrom,
                     const QByteArray& aTo, time_t aDate,
                     const MessageStatus& aStatus, const QByteArray& aXMark,
                     const QByteArray& replyToId, const QByteArray& replyToAuxId,
                     const QByteArray& msgId,
                     KMMsgEncryptionState encryptionState,
                     KMMsgSignatureState signatureState,
                     KMMsgMDNSentState mdnSentState,
                     const QByteArray& prefCharset,
                     off_t aFolderOffset, size_t aMsgSize,
                     size_t aMsgSizeServer, ulong aUID)
{
#ifdef KMAIL_SQLITE_INDEX
    mData = 0;
#else
    mIndexOffset = 0;
#endif
    mIndexLength = 0;
    if(!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers = KMMsgInfoPrivate::ALL_SET;
    kd->subject = decodeRFC2047String( aSubject, prefCharset );
    kd->from = decodeRFC2047String( StringUtil::stripEmailAddr( aFrom ),
                                    prefCharset );
    kd->to = decodeRFC2047String( StringUtil::stripEmailAddr( aTo ),
                                  prefCharset );
    kd->replyToIdMD5 = base64EncodedMD5( replyToId );
    kd->replyToAuxIdMD5 = base64EncodedMD5( replyToAuxId );
    kd->strippedSubjectMD5 = base64EncodedMD5( KMMessage::stripOffPrefixes( kd->subject ), true /*utf8*/ );
    kd->msgIdMD5 = base64EncodedMD5( msgId );
    kd->xmark = aXMark;
    kd->folderOffset = aFolderOffset;
    mStatus = aStatus;
    kd->msgSize = aMsgSize;
    kd->date = aDate;
    kd->file = "";
    kd->encryptionState = encryptionState;
    kd->signatureState = signatureState;
    kd->mdnSentState = mdnSentState;
    kd->msgSizeServer = aMsgSizeServer;
    kd->UID = aUID;
    mDirty     = false;
}

void KMMsgInfo::init(const QByteArray& aSubject, const QByteArray& aFrom,
                     const QByteArray& aTo, time_t aDate,
                     const MessageStatus& aStatus, const QByteArray& aXMark,
                     const QByteArray& replyToId, const QByteArray& replyToAuxId,
                     const QByteArray& msgId,
                     const QByteArray& aFileName,
                     KMMsgEncryptionState encryptionState,
                     KMMsgSignatureState signatureState,
                     KMMsgMDNSentState mdnSentState,
                     const QByteArray &prefCharset,
                     size_t aMsgSize,
                     size_t aMsgSizeServer, ulong aUID)
{
  // use the "normal" init for most stuff
  init( aSubject, aFrom, aTo, aDate, aStatus, aXMark, replyToId, replyToAuxId,
        msgId, encryptionState, signatureState, mdnSentState, prefCharset,
        (unsigned long)0, aMsgSize, aMsgSizeServer, aUID );
  kd->file = aFileName;
}


//-----------------------------------------------------------------------------
QString KMMsgInfo::subject(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::SUBJECT_SET)
        return kd->subject;
    return getStringPart(MsgSubjectPart);
}


//-----------------------------------------------------------------------------
QString KMMsgInfo::fromStrip(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::FROM_SET)
        return kd->from;
    return getStringPart(MsgFromPart);
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::fileName(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::FILE_SET)
        return kd->file;
    return getStringPart(MsgFilePart);
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::tagString( void ) const
{
    return tagList()->join( "," );
}
KMMessageTagList* KMMsgInfo::tagList( void ) const
{
  if (mTagList)
    return mTagList;
  mTagList = new KMMessageTagList( KMMessageTagList::split( ",",
                              getStringPart( MsgTagPart ) ) );
  mTagList->prioritySort();
  return mTagList;
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::toStrip(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::TO_SET)
        return kd->to;
    return getStringPart(MsgToPart);
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::xmark(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::XMARK_SET)
        return kd->xmark;
    return getStringPart(MsgXMarkPart);
}


//-----------------------------------------------------------------------------
QString KMMsgInfo::replyToIdMD5(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::REPLYTO_SET)
        return kd->replyToIdMD5;
    return getStringPart(MsgReplyToIdMD5Part);
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::replyToAuxIdMD5() const
{
    if( kd && kd->modifiers & KMMsgInfoPrivate::REPLYTOAUX_SET )
        return kd->replyToAuxIdMD5;
    return getStringPart( MsgReplyToAuxIdMD5Part );
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::strippedSubjectMD5() const
{
    if( kd && kd->modifiers & KMMsgInfoPrivate::STRIPPEDSUBJECT_SET )
        return kd->strippedSubjectMD5;
    return getStringPart( MsgStrippedSubjectMD5Part );
}


//-----------------------------------------------------------------------------
bool KMMsgInfo::subjectIsPrefixed() const
{
    return strippedSubjectMD5() != base64EncodedMD5( subject().trimmed(), true /*utf8*/ );
}

//-----------------------------------------------------------------------------
QString KMMsgInfo::msgIdMD5(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::MSGID_SET)
        return kd->msgIdMD5;
    return getStringPart(MsgIdMD5Part);
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setSubject(const QString& aSubject)
{
    if(aSubject == subject())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::SUBJECT_SET;
    kd->subject = aSubject;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setXMark(const QString& aXMark)
{
    if (aXMark == xmark())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::XMARK_SET;
    kd->xmark = aXMark;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setReplyToIdMD5(const QString& aReplyToIdMD5)
{
    if (aReplyToIdMD5 == replyToIdMD5())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::REPLYTO_SET;
    kd->replyToIdMD5 = aReplyToIdMD5;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setReplyToAuxIdMD5( const QString& aReplyToAuxIdMD5 )
{
    if( aReplyToAuxIdMD5 == replyToAuxIdMD5() )
        return;

    if( !kd )
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::REPLYTOAUX_SET;
    kd->replyToAuxIdMD5 = aReplyToAuxIdMD5;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::initStrippedSubjectMD5()
{
    if( kd && kd->modifiers & KMMsgInfoPrivate::STRIPPEDSUBJECT_SET )
        return;
    QString rawSubject = KMMessage::stripOffPrefixes( subject() );
    QString subjectMD5 = base64EncodedMD5( rawSubject, true /*utf8*/ );
    if( !kd )
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::STRIPPEDSUBJECT_SET;
    kd->strippedSubjectMD5 = subjectMD5;
    mDirty = true;
}


//-----------------------------------------------------------------------------
void KMMsgInfo::setMsgIdMD5(const QString& aMsgIdMD5)
{
    if (aMsgIdMD5 == msgIdMD5())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::MSGID_SET;
    kd->msgIdMD5 = aMsgIdMD5;
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setEncryptionState( const KMMsgEncryptionState s, int idx )
{
    if (s == encryptionState())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::ENCRYPTION_SET;
    kd->encryptionState = s;
    KMMsgBase::setEncryptionState(s, idx); //base does more "stuff"
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setSignatureState( const KMMsgSignatureState s, int idx )
{
    if (s == signatureState())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::SIGNATURE_SET;
    kd->signatureState = s;
    KMMsgBase::setSignatureState(s, idx); //base does more "stuff"
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setMDNSentState( const KMMsgMDNSentState s, int idx )
{
    if (s == mdnSentState())
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::MDN_SET;
    kd->mdnSentState = s;
    KMMsgBase::setMDNSentState(s, idx); //base does more "stuff"
    mDirty = true;
}

//-----------------------------------------------------------------------------
MessageStatus& KMMsgInfo::status() const
{
    if ( mStatus.isOfUnknownStatus() ) {
        mStatus.fromQInt32( getLongPart( MsgStatusPart ) );
        if ( mStatus.isOfUnknownStatus() ) {
            // We are opening an old index for the first time, get the legacy
            // status and merge it in.
            // This is kept to provide an upgrade path from the old single
            // status to the new multiple status scheme.
            KMLegacyMsgStatus legacyMsgStatus = (KMLegacyMsgStatus)getLongPart(MsgLegacyStatusPart);
            mStatus.setRead();
            switch (legacyMsgStatus) {
                case KMLegacyMsgStatusUnknown:
                    mStatus.clear();
                    break;
                case KMLegacyMsgStatusNew:
                    mStatus.setNew();
                    break;
                case KMLegacyMsgStatusUnread:
                    mStatus.setUnread();
                    break;
                case KMLegacyMsgStatusRead:
                    mStatus.setRead();
                    break;
                case KMLegacyMsgStatusOld:
                    mStatus.setOld();
                    break;
                case KMLegacyMsgStatusDeleted:
                    mStatus.setDeleted();
                    break;
                case KMLegacyMsgStatusReplied:
                    mStatus.setReplied();
                    break;
                case KMLegacyMsgStatusForwarded:
                    mStatus.setForwarded();
                    break;
                case KMLegacyMsgStatusQueued:
                    mStatus.setQueued();
                    break;
                case KMLegacyMsgStatusSent:
                    mStatus.setSent();
                    break;
                case KMLegacyMsgStatusFlag:
                    mStatus.setImportant();
                    break;
                default:
                    break;
            }

        }
    }
    return mStatus;
}


//-----------------------------------------------------------------------------
KMMsgEncryptionState KMMsgInfo::encryptionState() const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::ENCRYPTION_SET)
      return kd->encryptionState;
    unsigned long encState = getLongPart(MsgCryptoStatePart) & 0x0000FFFF;
    return encState ? (KMMsgEncryptionState)encState : KMMsgEncryptionStateUnknown;
}


KMMsgSignatureState KMMsgInfo::signatureState() const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::SIGNATURE_SET)
      return kd->signatureState;
    unsigned long sigState = getLongPart(MsgCryptoStatePart) >> 16;
    return sigState ? (KMMsgSignatureState)sigState : KMMsgSignatureStateUnknown;
}

KMMsgMDNSentState KMMsgInfo::mdnSentState() const {
    if (kd && kd->modifiers & KMMsgInfoPrivate::MDN_SET)
      return kd->mdnSentState;
    unsigned long mdnState = getLongPart(MsgMDNSentPart);
    return mdnState ? (KMMsgMDNSentState)mdnState : KMMsgMDNStateUnknown;
}


//-----------------------------------------------------------------------------
off_t KMMsgInfo::folderOffset(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::OFFSET_SET)
        return kd->folderOffset;
    return getLongPart(MsgOffsetPart);
}

//-----------------------------------------------------------------------------
size_t KMMsgInfo::msgSize(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::SIZE_SET)
        return kd->msgSize;
    return getLongPart(MsgSizePart);
}

//-----------------------------------------------------------------------------
time_t KMMsgInfo::date(void) const
{
    time_t res;
    if (kd && kd->modifiers & KMMsgInfoPrivate::DATE_SET)
      res = kd->date;
    else
      res = getLongPart(MsgDatePart);
    return res;
}

//-----------------------------------------------------------------------------
size_t KMMsgInfo::msgSizeServer(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::SIZESERVER_SET)
      return kd->msgSizeServer;
    return getLongPart(MsgSizeServerPart);
}

//-----------------------------------------------------------------------------
ulong KMMsgInfo::UID(void) const
{
    if (kd && kd->modifiers & KMMsgInfoPrivate::UID_SET)
      return kd->UID;
    return getLongPart(MsgUIDPart);
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setMsgSize(size_t sz)
{
    if (sz == msgSize())
        return;

    if(!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::SIZE_SET;
    kd->msgSize = sz;
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setMsgSizeServer(size_t sz)
{
    if (sz == msgSizeServer())
      return;

    if(!kd)
      kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::SIZESERVER_SET;
    kd->msgSizeServer = sz;
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setUID(ulong uid)
{
    if (uid == UID())
      return;

    if(!kd)
      kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::UID_SET;
    kd->UID = uid;
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setFolderOffset(off_t offs)
{
    if (folderOffset() == offs)
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::OFFSET_SET;
    kd->folderOffset = offs;
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setFileName(const QString& file)
{
    if (fileName() == file)
        return;

    if (!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::FILE_SET;
    kd->file = file;
    mDirty = true;
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setStatus(const MessageStatus& aStatus, int idx)
{
    if( aStatus == mStatus )
        return;
    KMMsgBase::setStatus( aStatus, idx ); //base does more "stuff"
}

//-----------------------------------------------------------------------------
void KMMsgInfo::setDate(time_t aUnixTime)
{
    if(aUnixTime == date())
        return;

    if(!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers |= KMMsgInfoPrivate::DATE_SET;
    kd->date = aUnixTime;
    mDirty = true;
}

//--- For compatibility with old index files
void KMMsgInfo::compat_fromOldIndexString(const QByteArray& str, bool toUtf8)
{
    const char *start, *offset;

    if(!kd)
        kd = new KMMsgInfoPrivate;
    kd->modifiers = KMMsgInfoPrivate::ALL_SET;
    kd->xmark   = str.mid(33, 3).trimmed();
    kd->folderOffset = str.mid(2,9).toULong();
    kd->msgSize = str.mid(12,9).toULong();
    kd->date = (time_t)str.mid(22,10).toULong();
    mStatus.setStatusFromStr( str );
    if (toUtf8) {
        kd->subject = str.mid(37, 100).trimmed();
        kd->from = str.mid(138, 50).trimmed();
        kd->to = str.mid(189, 50).trimmed();
    } else {
        start = offset = str.data() + 37;
        while (*start == ' ' && start - offset < 100) start++;
        kd->subject = QString::fromUtf8(str.mid(start - str.data(),
            100 - (start - offset)), 100 - (start - offset));
        start = offset = str.data() + 138;
        while (*start == ' ' && start - offset < 50) start++;
        kd->from = QString::fromUtf8(str.mid(start - str.data(),
            50 - (start - offset)), 50 - (start - offset));
        start = offset = str.data() + 189;
        while (*start == ' ' && start - offset < 50) start++;
        kd->to = QString::fromUtf8(str.mid(start - str.data(),
            50 - (start - offset)), 50 - (start - offset));
    }
    kd->replyToIdMD5 = str.mid(240, 22).trimmed();
    kd->msgIdMD5 = str.mid(263, 22).trimmed();
    mDirty = false;
}

bool KMMsgInfo::dirty(void) const
{
    if( KMMsgBase::dirty() )
      return true;
    return kd && kd->modifiers != KMMsgInfoPrivate::NONE_SET;
}
