// Author: Don Sanders <sanders@kde.org>
// License GPL


#include <config.h>

#include "kmstartup.h"

#include "kmkernel.h" //control center
#include "kcursorsaver.h"

#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <kcrash.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kaboutdata.h>

#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#undef Status // stupid X headers

extern "C" {

// Crash recovery signal handler
void kmsignalHandler(int sigId)
{
  kmsetSignalHandler(SIG_DFL);
  fprintf(stderr, "*** KMail got signal %d (Exiting)\n", sigId);
  // try to cleanup all windows
  if (kmkernel) kmkernel->dumpDeadLetters();
  ::exit(-1); //
}

// Crash recovery signal handler
void kmcrashHandler(int sigId)
{
  kmsetSignalHandler(SIG_DFL);
  fprintf(stderr, "*** KMail got signal %d (Crashing)\n", sigId);
  // try to cleanup all windows
  kmkernel->dumpDeadLetters();
  // Return to DrKonqi.
}
//-----------------------------------------------------------------------------


void kmsetSignalHandler(void (*handler)(int))
{
  signal(SIGKILL, handler);
  signal(SIGTERM, handler);
  signal(SIGHUP,  handler);
  KCrash::setEmergencySaveFunction(kmcrashHandler);
}

}
//-----------------------------------------------------------------------------

namespace {
  QString getMyHostName() {
    char hostNameC[256];
    // null terminate this C string
    hostNameC[255] = 0;
    // set the string to 0 length if gethostname fails
    if(gethostname(hostNameC, 255))
      hostNameC[0] = 0;
    return QString::fromLocal8Bit(hostNameC);
  }
} // anon namespace

namespace KMail {

void checkConfigUpdates() {
  static const char * const updates[] = {
    "9",
    "3.1-update-identities",
    "3.1-use-identity-uoids",
    "3.1-new-mail-notification",
    "3.2-update-loop-on-goto-unread-settings",
    "3.1.4-dont-use-UOID-0-for-any-identity",
    "3.2-misc",
    "3.2-moves",
    "3.3-use-ID-for-accounts",
    "3.3-update-filter-rules"
  };
  static const int numUpdates = sizeof updates / sizeof *updates;

  KConfig * config = KMKernel::config();
  KConfigGroup startup( config, "Startup" );
  const int configUpdateLevel = startup.readNumEntry( "update-level", 0 );
  if ( configUpdateLevel == numUpdates ) // Optimize for the common case that everything is OK
    return;

  for ( int i = 0 ; i < numUpdates ; ++i )
    config->checkUpdate( updates[i], "kmail.upd" );
  startup.writeEntry( "update-level", numUpdates );
}

void lockOrDie() {
// Check and create a lock file to prevent concurrent access to kmail files
  QString appName = kapp->instanceName();
  if ( appName.isEmpty() )
    appName = "kmail";

  QString programName;
  const KAboutData *about = kapp->aboutData();
  if ( about )
    programName = about->programName();
  if ( programName.isEmpty() )
    programName = i18n("KMail");

  QString lockLocation = locateLocal("data", "kmail/lock");
  KSimpleConfig config(lockLocation);
  int oldPid = config.readNumEntry("pid", -1);
  const QString oldHostName = config.readEntry("hostname");
  const QString oldAppName = config.readEntry( "appName", appName );
  const QString oldProgramName = config.readEntry( "programName", programName );
  const QString hostName = getMyHostName();

  if ( oldPid != -1 && oldHostName != hostName ) {
    // KUniqueApplication will take care of the case oldHostName == hostName
    QString msg;
    if ( oldAppName == appName )
      msg = i18n("%1 already seems to be running on %2. Running %1 more "
                 "than once can cause the loss of mail. You should not "
                 "start %1 on this computer unless you are sure that it is "
                 "not already running on %2.")
            .arg( programName, oldHostName );
    else
      msg = i18n("%1 seems to be running on %3. Running %1 and %2 at the "
                 "same time can cause the loss of mail. You should not "
                 "start %2 on this computer unless you are sure that %1 is "
                 "not running on %3.")
            .arg( oldProgramName, programName, oldHostName );

    KCursorSaver idle( KBusyPtr::idle() );
    if ( KMessageBox::No ==
         KMessageBox::warningYesNo( 0, msg, QString::null,
                                    i18n("Start %1").arg( programName ),
                                    i18n("Exit") ) ) {
      exit(1);
    }
  }

  config.writeEntry("pid", getpid());
  config.writeEntry("hostname", hostName);
  config.writeEntry( "appName", appName );
  config.writeEntry( "programName", programName );
  config.sync();
}

void insertLibraryCatalogues() {
  static const char * const catalogues[] = {
    "libkdenetwork",
    "libkdepim",
    "libktnef",
    "libkcal",
    "libksieve",
  };

  KLocale * l = KGlobal::locale();
  for ( unsigned int i = 0 ; i < sizeof catalogues / sizeof *catalogues ; ++i )
    l->insertCatalogue( catalogues[i] );
}

void cleanup()
{
  const QString lockLocation = locateLocal("data", "kmail/lock");
  KSimpleConfig config(lockLocation);
  config.writeEntry("pid", -1);
  config.sync();
}
}
