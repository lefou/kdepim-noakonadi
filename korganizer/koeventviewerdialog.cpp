// $Id$

#include <klocale.h>

#include "koevent.h"
#include "koeventviewer.h"

#include "koeventviewerdialog.h"
#include "koeventviewerdialog.moc"


KOEventViewerDialog::KOEventViewerDialog(QWidget *parent,const char *name)
  : KDialogBase(parent,name,false,i18n("Event Viewer"),Ok,Ok,false,
                i18n("Edit"))
{
  mEventViewer = new KOEventViewer(this);
  setMainWidget(mEventViewer);

  setMinimumSize(300,200);
}

KOEventViewerDialog::~KOEventViewerDialog()
{
}

void KOEventViewerDialog::setEvent(KOEvent *event)
{
  mEventViewer->setEvent(event);
}

void KOEventViewerDialog::setTodo(KOEvent *event)
{
  mEventViewer->setTodo(event);
}
