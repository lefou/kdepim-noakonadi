/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KMREADERWIN_H
#define KMREADERWIN_H

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>
#include <QList>
#include <QResizeEvent>
#include <kurl.h>
#include <kservice.h>
#include <messagecore/messagestatus.h>
#include <kvbox.h>
using KPIM::MessageStatus;
#include "kmmimeparttree.h" // Needed for friend declaration.
#include "interfaces/observer.h"

#include <map>

class QSplitter;
class KHBox;
class QTreeWidgetItem;
class QString;
class QTextCodec;


class KActionCollection;
class KAction;
class KSelectAction;
class KToggleAction;
class KToggleAction;
class KHTMLPart;
class KUrl;

class KMFolder;
class KMMessage;
class KMMessagePart;
namespace KMail {
  namespace Interface {
    class Observable;
    class BodyPartMemento;
  }
  class ObjectTreeParser;
  class AttachmentStrategy;
  class HeaderStrategy;
  class HeaderStyle;
  class HtmlWriter;
  class KHtmlPartHtmlWriter;
  class HtmlStatusBar;
  class CSSHelper;
}

class partNode; // might be removed when KMime is used instead of mimelib
                //                                      (khz, 29.11.2001)

namespace KParts {
  struct BrowserArguments;
  class OpenUrlArguments;
}

namespace DOM {
  class HTMLElement;
}

/**
   This class implements a "reader window", that is a window
   used for reading or viewing messages.
*/

class KMReaderWin: public QWidget, public KMail::Interface::Observer {
  Q_OBJECT

  friend void KMMimePartTree::slotItemClicked( QTreeWidgetItem* );
  friend void KMMimePartTree::slotContextMenuRequested( const QPoint & );
  friend void KMMimePartTree::slotSaveAs();
  friend void KMMimePartTree::startDrag( Qt::DropActions actions );

  friend class KMail::ObjectTreeParser;
  friend class KMail::KHtmlPartHtmlWriter;

public:
  KMReaderWin( QWidget *parent, QWidget *mainWindow,
               KActionCollection *actionCollection, Qt::WindowFlags f = 0 );
  virtual ~KMReaderWin();

  /**
     \reimp from Interface::Observer
     Updates the current message
   */
  void update( KMail::Interface::Observable * );

  /** Read settings from app's config file. */
  void readConfig();

  /** Write settings to app's config file. Calls sync() if withSync is true. */
  void writeConfig( bool withSync=true ) const;

  const KMail::HeaderStyle * headerStyle() const {
    return mHeaderStyle;
  }
  /** Set the header style and strategy. We only want them to be set
      together. */
  void setHeaderStyleAndStrategy( const KMail::HeaderStyle * style,
                                  const KMail::HeaderStrategy * strategy );

  /** Getthe message header strategy. */
  const KMail::HeaderStrategy * headerStrategy() const {
    return mHeaderStrategy;
  }

  /** Get/set the message attachment strategy. */
  const KMail::AttachmentStrategy * attachmentStrategy() const {
    return mAttachmentStrategy;
  }
  void setAttachmentStrategy( const KMail::AttachmentStrategy * strategy );

  /** Get selected override character encoding.
      @return The encoding selected by the user or an empty string if auto-detection
      is selected. */
  QString overrideEncoding() const { return mOverrideEncoding; }

  /** Set the override character encoding. */
  void setOverrideEncoding( const QString & encoding );

  /** Get codec corresponding to the currently selected override character encoding.
      @return The override codec or 0 if auto-detection is selected. */
  const QTextCodec * overrideCodec() const;

  /** Set printing mode */
  virtual void setPrinting(bool enable) { mPrinting = enable; }

  /** Set the message that shall be shown. If msg is 0, an empty page is
      displayed. */
  virtual void setMsg( KMMessage* msg, bool force = false );

  /**
   * This should be called when setting a message that was constructed from another message, which
   * is the case when viewing encapsulated messages in the separate reader window.
   * We need to know the serial number of the original message, and at which part index the encapsulated
   * message was at that original message, so that deleting and editing attachments can work on the
   * original message.
   *
   * This is a HACK. There really shouldn't be a copy of the original mail.
   *
   * @see slotDeleteAttachment, slotEditAttachment, fillCommandInfo
   */
  void setOriginalMsg( unsigned long serNumOfOriginalMessage, int nodeIdOffset );

  /** Instead of settings a message to be shown sets a message part
      to be shown */
  void setMsgPart( KMMessagePart* aMsgPart, bool aHTML,
                   const QString& aFileName, const QString& pname );

  void setMsgPart( partNode * node );

  /** Show or hide the Mime Tree Viewer if configuration
      is set to smart mode.  */
  void showHideMimeTree();

  /** Store message id of last viewed message,
      normally no need to call this function directly,
      since correct value is set automatically in
      parseMsg(KMMessage* aMsg, bool onlyProcessHeaders). */
  void setIdOfLastViewedMessage( const QString & msgId )
    { mIdOfLastViewedMessage = msgId; }

  /** Clear the reader and discard the current message. */
  void clear(bool force = false) { setMsg(0, force); }

  /** Saves the relative position of the scroll view. Call this before calling update()
      if you want to preserve the current view. */
  void saveRelativePosition();

  /** Re-parse the current message. */
  void update(bool force = false);

  /** Print message. */
  virtual void printMsg(  KMMessage* aMsg );

  /** Return selected text */
  QString copyText();

  /** Get/set auto-delete msg flag. */
  bool autoDelete(void) const { return mAutoDelete; }
  void setAutoDelete(bool f) { mAutoDelete=f; }

  /** Override default html mail setting */
  bool htmlOverride() const { return mHtmlOverride; }
  void setHtmlOverride( bool override );

  /** Override default load external references setting */
  bool htmlLoadExtOverride() const { return mHtmlLoadExtOverride; }
  void setHtmlLoadExtOverride( bool override );

  /** Is html mail to be supported? Takes into account override */
  bool htmlMail();

  /** Is loading ext. references to be supported? Takes into account override */
  bool htmlLoadExternal();

  /** Returns the MD5 hash for the list of new features */
  static QString newFeaturesMD5();

  /** Display a generic HTML splash page instead of a message */
  void displaySplashPage( const QString &info );

  /** Display the about page instead of a message */
  void displayAboutPage();

  /** Display the 'please wait' page instead of a message */
  void displayBusyPage();
  /** Display the 'we are currently in offline mode' page instead of a message */
  void displayOfflinePage();

  /** Enable the displaying of messages again after an URL was displayed */
  void enableMsgDisplay();

  /**
   * View message part of type message/RFC822 in extra viewer window.
   * @param msgPart the part to display
   * @param nodeId the part index of the message part that is displayed
   */
  void atmViewMsg( KMMessagePart* msgPart, int nodeId );

  bool atBottom() const;

  bool isFixedFont() { return mUseFixedFont; }
  void setUseFixedFont( bool useFixedFont );

  /** Return the HtmlWriter connected to the KHTMLPart we use */
  KMail::HtmlWriter * htmlWriter() { return mHtmlWriter; }

  // Action to reply to a message
  // but action( "some_name" ) some name could be used instead.
  KToggleAction *toggleFixFontAction() { return mToggleFixFontAction; }
  KToggleAction *toggleDisplayModeAction() { return mToggleDisplayModeAction; }
  KAction *mailToComposeAction() { return mMailToComposeAction; }
  KAction *mailToReplyAction() { return mMailToReplyAction; }
  KAction *mailToForwardAction() { return mMailToForwardAction; }
  KAction *addAddrBookAction() { return mAddAddrBookAction; }
  KAction *openAddrBookAction() { return mOpenAddrBookAction; }
  KAction *copyAction() { return mCopyAction; }
  KAction *selectAllAction() { return mSelectAllAction; }
  KAction *copyURLAction() { return mCopyURLAction; }
  KAction *urlOpenAction() { return mUrlOpenAction; }
  KAction *urlSaveAsAction() { return mUrlSaveAsAction; }
  KAction *addBookmarksAction() { return mAddBookmarksAction;}
  KAction *toggleMimePartTreeAction() { return mToggleMimePartTreeAction; }
  // This function returns the complete data that were in this
  // message parts - *after* all encryption has been removed that
  // could be removed.
  // - This is used to store the message in decrypted form.
  void objectTreeToDecryptedMsg( partNode* node,
                                 QByteArray& resultingData,
                                 KMMessage& theMessage,
                                 bool weAreReplacingTheRootNode = false,
                                 int recCount = 0 );

  /** Returns message part from given URL or null if invalid. */
  partNode* partNodeFromUrl(const KUrl &url);

  partNode * partNodeForId( int id );

  KUrl tempFileUrlFromPartNode( const partNode *node );

  /** Returns id of message part from given URL or -1 if invalid. */
  static int msgPartFromUrl(const KUrl &url);

  void setUpdateAttachment( bool update = true ) { mAtmUpdate = update; }

  /** Access to the KHTMLPart used for the viewer. Use with
      care! */
  KHTMLPart * htmlPart() const { return mViewer; }

  /** Returns the current message or 0 if none. */
  KMMessage* message() const;

  void openAttachment( int id, const QString & name );
  void saveAttachment( const KUrl &tempFileName );

  void emitUrlClicked( const KUrl & url, int button ) {
    emit urlClicked( url, button );
  }

  void emitPopupMenu( const KUrl & url, const QPoint & p ) {
    if ( message() )
      emit popupMenu( *message(), url, p );
  }

  /**
   * Sets the current attachment ID and the current attachment temporary filename
   * to the given values.
   * Call this so that slotHandleAttachment() knows which attachment to handle.
   */
  void prepareHandleAttachment( int id, const QString& fileName );

  void showAttachmentPopup( int id, const QString & name, const QPoint & p );

  /** Set the serial number of the message this reader window is currently
   *  waiting for. Used to discard updates for already deselected messages. */
  void setWaitingForSerNum( unsigned long serNum ) { mWaitingForSerNum = serNum; }

  QWidget* mainWindow() { return mMainWindow; }

  /** Returns whether the message should be decryted. */
  bool decryptMessage() const;

  /** Enforce message decryption. */
  void setDecryptMessageOverwrite( bool overwrite = true ) { mDecrytMessageOverwrite = overwrite; }

  /** Show signature details. */
  bool showSignatureDetails() const { return mShowSignatureDetails; }

  /** Show signature details. */
  void setShowSignatureDetails( bool showDetails = true ) { mShowSignatureDetails = showDetails; }

  KMail::CSSHelper* cssHelper() const;

  /* show or hide the list that points to the attachments */
  bool showAttachmentQuicklist() const { return mShowAttachmentQuicklist; }

  /* show or hide the list that points to the attachments */
  void setShowAttachmentQuicklist( bool showAttachmentQuicklist = true ) { mShowAttachmentQuicklist = showAttachmentQuicklist; }

  /** Get the HTMLElement with id id */
  DOM::HTMLElement getHTMLElementById( const QString &id );

  /** Return weather to show or hide the full list of "To" addresses */
  bool showFullToAddressList() const;

  /** Show or hide the full list of "To" addresses */
  void setShowFullToAddressList( bool showFullToAddressList = true );

  /** Return weather to show or hide the full list of "To" addresses */
  bool showFullCcAddressList() const;

  /** Show or hide the full list of "To" addresses */
  void setShowFullCcAddressList( bool showFullCcAddressList = true );

  /* retrieve BodyPartMemento of id \a which for partNode \a node */
  KMail::Interface::BodyPartMemento * bodyPartMemento( const partNode * node, const QByteArray & which ) const;

  /* set/replace BodyPartMemento \a memento of id \a which for
     partNode \a node. If there was a BodyPartMemento registered
     already, replaces (deletes) that one. */
  void setBodyPartMemento( const partNode * node, const QByteArray & which, KMail::Interface::BodyPartMemento * memento );

  /// Scrolls to the given attachment and marks it with a yellow border
  void scrollToAttachment( const partNode *node );

private:
  /* deletes all BodyPartMementos. Use this when skipping to another
     message (as opposed to re-loading the same one again). */
  void clearBodyPartMementos();

signals:
  /** Emitted after parsing of a message to have it stored
      in unencrypted state in it's folder. */
  void replaceMsgByUnencryptedVersion();

  /** The user presses the right mouse button. 'url' may be 0. */
  void popupMenu(KMMessage &msg, const KUrl &url, const QPoint& mousePos);

  /** The user has clicked onto an URL that is no attachment. */
  void urlClicked(const KUrl &url, int button);

  /** Pgp displays a password dialog */
  void noDrag(void);

public slots:

  /** Select message body. */
  void selectAll();

  /** Force update even if message is the same */
  void clearCache();

  /** Refresh the reader window */
  void updateReaderWin();

  /** HTML Widget scrollbar and layout handling. */
  void slotScrollUp();
  void slotScrollDown();
  void slotScrollPrior();
  void slotScrollNext();
  void slotJumpDown();
  void slotDocumentChanged();
  void slotDocumentDone();
  void slotTextSelected(bool);

  /** An URL has been activate with a click. */
  void slotUrlOpen(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);

  /** The mouse has moved on or off an URL. */
  void slotUrlOn(const QString &url);

  /** The user presses the right mouse button on an URL. */
  void slotUrlPopup(const QString &, const QPoint& mousePos);

  /** The user selected "Find" from the menu. */
  void slotFind();

  /** The user toggled the "Fixed Font" flag from the view menu. */
  void slotToggleFixedFont();

  void slotToggleMimePartTree();

  /** Copy the selected text to the clipboard */
  void slotCopySelectedText();

   void slotUrlClicked();

  /** Operations on mailto: URLs. */
  void slotMailtoReply();
  void slotMailtoCompose();
  void slotMailtoForward();
  void slotMailtoAddAddrBook();
  void slotMailtoOpenAddrBook();
  /** Copy URL in mUrlCurrent to clipboard. Removes "mailto:" at
      beginning of URL before copying. */
  void slotUrlCopy();
  void slotUrlOpen( const KUrl &url = KUrl() );
  /** Save the page to a file */
  void slotUrlSave();
  void slotAddBookmarks();
  void slotSaveMsg();
  void slotSaveAttachments();

  void slotMessageArrived( KMMessage *msg );

  void slotLevelQuote( int l );
  void slotTouchMessage();

  /**
   * Find the node ID and the message of the attachment that should be edited or deleted.
   * This is used when setOriginalMsg() was called before, in that case we want to operate
   * on the original message instead of our copy.
   *
   * @see setOriginalMsg
   */
  void fillCommandInfo( partNode *node, KMMessage **msg, int *nodeId );

  void slotDeleteAttachment( partNode* node );
  void slotEditAttachment( partNode* node );

  /**
   * Does an action for the current attachment.
   * The action is defined by the KMHandleAttachmentCommand::AttachmentAction
   * enum.
   * prepareHandleAttachment() needs to be called before calling this to set the
   * correct attachment ID.
   */
  void slotHandleAttachment( int action );

  /** Toggle display mode between HTML and plain text. */
  void slotToggleHtmlMode();

protected slots:
  void slotCycleHeaderStyles();
  void slotBriefHeaders();
  void slotFancyHeaders();
  void slotEnterpriseHeaders();
  void slotStandardHeaders();
  void slotLongHeaders();
  void slotAllHeaders();

  void slotCycleAttachmentStrategy();
  void slotIconicAttachments();
  void slotSmartAttachments();
  void slotInlineAttachments();
  void slotHideAttachments();

  /** Some attachment operations. */
  void slotAtmView( int id, const QString& name );
  void slotDelayedResize();

  /** Print message. Called on as a response of finished() signal of mPartHtmlWriter
      after rendering is finished.
      In the very end it deletes the KMReaderWin window that was created
      for the purpose of rendering. */
  void slotPrintMsg();

protected:
  /** reimplemented in order to update the frame width in case of a changed
      GUI style */
  void styleChange( QStyle& oldStyle );

  /** Set the width of the frame to a reasonable value for the current GUI
      style */
  void setStyleDependantFrameWidth();

  /** Watch for palette changes */
  virtual bool event(QEvent *e);

  /** Calculate the pixel size */
  int pointsToPixel(int pointSize) const;

  /** Feeds the HTML viewer with the contents of the given message.
    HTML begin/end parts are written around the message. */
  void displayMessage();

  /** Parse given message and add it's contents to the reader window. */
  virtual void parseMsg( KMMessage* msg  );

  /** Creates a nice mail header depending on the current selected
    header style. */
  QString writeMsgHeader( KMMessage* aMsg, partNode *vCardNode = 0, bool topLevel = false );

  /** Writes the given message part to a temporary file and returns the
      name of this file or QString() if writing failed.
  */
  QString writeMessagePartToTempFile( KMMessagePart* msgPart, int partNumber );

  /**
    Creates a temporary dir for saving attachments, etc.
    Will be automatically deleted when another message is viewed.
    @param param Optional part of the directory name.
  */
  QString createTempDir( const QString &param = QString() );

  /** show window containing information about a vCard. */
  void showVCard(KMMessagePart *msgPart);

  /** HTML initialization. */
  virtual void initHtmlWidget(void);

  /** Some necessary event handling. */
  virtual void closeEvent(QCloseEvent *);
  virtual void resizeEvent(QResizeEvent *);

  /** Cleanup the attachment temp files */
  virtual void removeTempFiles();

  /** Event filter */
  bool eventFilter( QObject *obj, QEvent *ev );

private slots:
  void slotSetEncoding();
  void injectAttachments();

  /** Show hide all fields specified inside this function */
  void toggleFullAddressList();

private:
  void adjustLayout();
  void createWidgets();
  void createActions();
  void saveSplitterSizes() const;

  KToggleAction * actionForHeaderStyle( const KMail::HeaderStyle *,
                                       const KMail::HeaderStrategy * );
  KToggleAction * actionForAttachmentStrategy( const KMail::AttachmentStrategy * );
  /** Read override codec from configuration */
  void readGlobalOverrideCodec();

  QString renderAttachments( partNode *node, const QColor &bgColor );

  /** Show/Hide the field with id "field" */
  void toggleFullAddressList(const QString& field);

private:
  bool mHtmlMail, mHtmlLoadExternal, mHtmlOverride, mHtmlLoadExtOverride;
  int mAtmCurrent;
  QString mAtmCurrentName;
  KMMessage *mMessage;

  // See setOriginalMsg() for an explaination for those two.
  unsigned long mSerNumOfOriginalMessage;
  int mNodeIdOffset;

  // widgets:
  QSplitter * mSplitter;
  KHBox *mBox;
  KMail::HtmlStatusBar *mColorBar;
  KMMimePartTree* mMimePartTree;
  KHTMLPart *mViewer;

  const KMail::AttachmentStrategy * mAttachmentStrategy;
  const KMail::HeaderStrategy * mHeaderStrategy;
  const KMail::HeaderStyle * mHeaderStyle;
  bool mAutoDelete;
  /** where did the user save the attachment last time */
  QString mSaveAttachDir;
  static const int delay;
  QTimer mUpdateReaderWinTimer;
  QTimer mResizeTimer;
  QTimer mDelayedMarkTimer;
  QString mOverrideEncoding;
  QString mOldGlobalOverrideEncoding; // used to detect changes of the global override character encoding
  bool mMsgDisplay;
  bool mNoMDNsWhenEncrypted;
  unsigned long mLastSerNum;

  KMail::CSSHelper * mCSSHelper;
  bool mUseFixedFont;
  bool mPrinting;
  //bool mShowCompleteMessage;
  QStringList mTempFiles;
  QStringList mTempDirs;
  QList<int> mSplitterSizes;
  partNode* mRootNode;
  QString mIdOfLastViewedMessage;
  QWidget *mMainWindow;
  KActionCollection *mActionCollection;
  KAction *mMailToComposeAction, *mMailToReplyAction, *mMailToForwardAction,
      *mAddAddrBookAction, *mOpenAddrBookAction, *mCopyAction, *mCopyURLAction,
      *mUrlOpenAction, *mUrlSaveAsAction, *mAddBookmarksAction, *mSelectAllAction,
      *mScrollUpAction, *mScrollDownAction, *mScrollUpMoreAction, *mScrollDownMoreAction,
      *mToggleMimePartTreeAction;
  KSelectAction *mSelectEncodingAction;
  KToggleAction *mToggleFixFontAction, *mToggleDisplayModeAction;

  KUrl mHoveredUrl;
  KUrl mClickedUrl;
  QPoint mLastClickPosition;
  bool mCanStartDrag;

  KMail::HtmlWriter * mHtmlWriter;
  /** Used only to be able to connect and disconnect finished() signal
      in printMsg() and slotPrintMsg() since mHtmlWriter points only to abstract non-QObject class. */
  QPointer<KMail::KHtmlPartHtmlWriter> mPartHtmlWriter;

  std::map<QByteArray,KMail::Interface::BodyPartMemento*> mBodyPartMementoMap;
  // an attachment should be updated
  bool mAtmUpdate;
  int mChoice;
  unsigned long mWaitingForSerNum;
  float mSavedRelativePosition;
  int mLevelQuote;
  bool mDecrytMessageOverwrite;
  bool mShowSignatureDetails;
  bool mShowAttachmentQuicklist;
  bool mExternalWindow;
  bool mShowFullToAddressList;
  bool mShowFullCcAddressList;
};


#endif

